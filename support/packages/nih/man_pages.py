import re
import os
import html
from typing import Any, Callable, Mapping, MutableSequence
from markdown import Extension, Markdown
from markdown.blockprocessors import BlockProcessor
from xml.etree import ElementTree
from xml.etree.ElementTree import Element

class ManPageProcessor(BlockProcessor):
    """A markdown block processor that processes man page blocks, inserting the man page name and section as a heading and the man page body as a div with class "manpage"."""

    class _ManPage:
        """Stores the name, section, synopsis, and body of a man page."""
        def __init__(self, name: str|None = None, section: str|None = None, synopsis: str|None = None, body: Element|None = None):
            """Initializes a _ManPage instance.
            
            Arguments:
                name: The name of the man page, e.g. "zip_open".
                section: The section of the man page, e.g. "3"."
                synopsis: The synopsis line of the man page, e.g. "open a zip archive".
                body: The XML element representing the man page body.
            """
            self.name = name
            self.section = section
            self.synopsis = synopsis
            self.body = body
        
        
    _trigger_pattern = re.compile(r"^!man\s+")
    """The pattern that triggers the man page processor."""

    _comment_pattern = re.compile(r"<!--.*?-->", re.DOTALL)
    """The pattern that matches HTML comments."""

    _section_pattern = re.compile(r"\((\d+)\)", re.IGNORECASE)
    """The pattern that matches the section number in the man page title."""

    
    def __init__(self, md: Markdown, config: Mapping[str, Any]):
        """Initializes a ManPageProcessor instance.
        
        Arguments:
            md: The markdown instance.
            config: The configuration for the man page processor.
        """
        self.md = md
        self.config = config

    def test(self, parent, block):
        """Check if the block is a man page block.
        
        Arguments:
            parent: The parent element.
            block: The block to test.
            
        Returns:
            True if the block is a man page block, False otherwise.
        """
        return bool(self._trigger_pattern.match(block))

    def run(self, parent: Element, blocks: MutableSequence[str]) -> bool:
        """Process a man page block.
        
        Arguments:
            parent: The parent element.
            blocks: The list of blocks to process.
            
        Returns:
            True if the block was processed, False otherwise.
        """
        block = blocks[0]
        match = self._trigger_pattern.match(block)
        if not match:
            return False
        
        file = block[match.end():].strip()
        if not file:
            # TODO: raise "missing man page name" error
            return False

        manpage = self._process(file)
        if manpage is None:
            # TODO: raise "man page not found" error
            return False
        if manpage.body is None:
            # TODO: raise "man page body not found" error
            return False
        
        self.md.parser.parseBlocks(parent, [f"# {manpage.name}({manpage.section})"])
        parent.append(manpage.body)
        blocks.pop(0)
        return True
    
    def _find_file(self, name: str) -> str|None:
        """Finds the man page file for the given name in the configured directories.

        Arguments:
            name: The name of the man page, e.g. "zip_open".

        Returns:
            The path to the man page file if found, None otherwise.
        """
        for directory in self.config.get("path", []):
            file = os.path.join(directory, f"{name}.html")
            if os.path.isfile(file):
                return file
        return None

    def _process(self, name: str) -> _ManPage:
        """Processes a man page file and returns a _ManPage instance.

        Arguments:
            name: The name of the man page, e.g. "zip_open".

        Returns:
            A _ManPage instance if the file was processed successfully, None otherwise.
        """
        skip = True
        synopsis_tag = None
        synopsis = ""
        manpage = self._ManPage(name=name)

        file = self._find_file(name)
        if not file:
            raise FileNotFoundError(f"man page file for {name} not found")

        with open(file, "r") as f:
            html = f.read()
        html = self._comment_pattern.sub("", html)

        root = ElementTree.fromstring(html)
        title = root.find(".//title")
        if title is not None and title.text:
            title_text = title.text.strip()
            section_match = self._section_pattern.search(title_text)
            if section_match:
                manpage.section = section_match.group(1)
        elements = root.findall('.//div[@class="manual-text"]')

        if not elements:
            raise ValueError(f"man page file for {name} does not contain a div with class 'manual-text'")
        
        if len(elements) > 1:
            raise ValueError(f"man page file for {name} contains multiple divs with class 'manual-text'")
        
        page = elements[0]

        # Fix cross reference links.
        xrefs = page.findall('.//a[@class="Xr"]')
        for xref in xrefs:
            href = xref.get("href")
            if href and href.endswith(".html"):
                href = href[:-5] + ".md"
                xref.set("href", href)

        # Remove permalinks.
        self._remove_permalinks(page)

        # Fix section headings.
        section_headings = page.findall('.//h1[@class="Sh"]') + page.findall('.//h2[@class="Ss"]')
        for heading in section_headings:
            if heading.text:
                text = heading.text.strip().title()
                heading.text = text
            for child in heading:
                if child.tail:
                    text = child.tail.strip().title()
                    child.tail = text

        manpage.body = page
        return manpage
    
    def _remove_permalinks(self, page: Element) -> None:
        """Removes permalink anchors from the given page.

        Arguments:
            page: The page to remove permalinks from.
        """
        def is_permalink(element: Element) -> bool:
            if element.tag != "a":
                return False
            classes = element.get("class", "").split()
            if not classes:
                return False
            return "permalink" in classes or "In" in classes
        
        self._walk_tree(page, self._unwrap_element, is_permalink)
    
    def _walk_tree(self, element: Element, func: Callable[[Element, Element|None], None], predicate: Callable[[Element], bool]|None = None, parent: Element|None = None) -> None:
        """Walks the given element and its descendants, applying the given function to each element.

        Arguments:
            element: The element to walk.
            func: The function to apply to elements matching the predicate. It should take an Element and its parent as arguments.
            predicate: A function that takes an Element and returns True if the function should be applied to that element, False otherwise.
            parent: The parent of the current element, or None if the current element is the root.
        """
        if predicate is None or predicate(element):
            func(element, parent)
        for child in element:
            self._walk_tree(child, func, predicate, element)
    
    def _unwrap_element(self, element: Element, parent: Element|None) -> None:
        """Unwraps the given element, replacing it with its content in the parent.

        Arguments:
            element: The element to unwrap.
            parent: The parent of the element to unwrap.
        """
        if parent is None:
            return

        # Why simply taking text from one element and assigning it to text in another messes up entity serialization is beyond me. This workaround seems to fix it, but really shouldn't be necessary.
        def normalize_text_entities(value: str) -> str:
            return html.escape(html.unescape(value), quote=False)
        
        children = list(parent)
        text = element.text or ""
        content = list(element)
        if element.tail:
            if content:
                last = content[-1]
                last.tail = (last.tail or "") + element.tail
            else:
                text += element.tail

        index = children.index(element)
        parent[index:index+1] = content
        if text:
            if index > 0:
                previous = children[index - 1]
                previous.tail = normalize_text_entities((previous.tail or "") + text)
            else:
                parent.text = normalize_text_entities((parent.text or "") + text)

class ManPageExtension(Extension):
    """A markdown extension that adds support for man page blocks."""

    def __init__(self, *args, **kwargs):
        """Initializes a ManPageExtension instance."""
        self.config = kwargs

    def extendMarkdown(self, md):
        """Registers the ManPageProcessor with the markdown parser."""

        md.registerExtension(self)
        md.parser.blockprocessors.register(ManPageProcessor(md, self.config), "man_page", 25)

def makeExtension(*args, **kwargs):
    return ManPageExtension(*args, **kwargs)
