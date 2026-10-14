import re
import os
import html
from typing import Any, Callable, Mapping, MutableSequence
from markdown import Extension, Markdown
from markdown.blockprocessors import BlockProcessor
from xml.etree import ElementTree
from xml.etree.ElementTree import Element

class DownloadProcessor(BlockProcessor):
    """A markdown block processor that adds download links."""

    _trigger_pattern = re.compile(r"^!download\s+")
    """The pattern that triggers the download processor."""

    
    def __init__(self, md: Markdown, config: Mapping[str, Any]):
        """Initializes a DownloadProcessor instance.
        
        Arguments:
            md: The markdown instance.
            config: The configuration for the download processor.
        """
        self.md = md
        self.config = config
        self.url_prefix = config.get("url_prefix", "")
        if self.url_prefix and not self.url_prefix.endswith("/"):
            self.url_prefix += "/"

    def test(self, parent, block):
        """Check if the block is a download block.
        
        Arguments:
            parent: The parent element.
            block: The block to test.
            
        Returns:
            True if the block is a download block, False otherwise.
        """
        return bool(self._trigger_pattern.match(block))

    def run(self, parent: Element, blocks: MutableSequence[str]) -> bool:
        """Process a download block.
        
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
        
        arguments = block[match.end():].strip().split()
        if not arguments:
            raise ValueError("missing version argument")
        version = arguments[0].strip()
        if len(arguments) > 1:
            formats = arguments[1:]
        else:
            formats = ["xz", "gz"]

        self.md.parser.parseBlocks(parent, [f"Download release:"])
        for format in formats:
            filename = f"libzip-{version}.tar.{format}"
            self.md.parser.parseBlocks(parent, [f"- [{filename}]({self.url_prefix}{filename})"])
        blocks.pop(0)
        return True
    

class DownloadExtension(Extension):
    """A markdown extension that adds support for download blocks."""

    def __init__(self, *args, **kwargs):
        """Initializes a DownloadExtension instance."""
        self.config = kwargs

    def extendMarkdown(self, md):
        """Registers the DownloadProcessor with the markdown parser."""

        md.registerExtension(self)
        md.parser.blockprocessors.register(DownloadProcessor(md, self.config), "nih-download", 25)

def makeExtension(*args, **kwargs):
    return DownloadExtension(*args, **kwargs)
