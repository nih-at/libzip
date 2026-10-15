from . import download
from . import man_pages

from markdown import Extension

class NiHExtension(Extension):
    _extensions = {
        "man_pages": man_pages.ManPageExtension,
        "download": download.DownloadExtension,
    }

    def __init__(self, *args, **kwargs):
        self.args = args
        self.kwargs = kwargs
    
    def extendMarkdown(self, md):
        for (name, config) in self.kwargs.items():
            if name not in self._extensions:
                raise ValueError(f"unknown extension {name}")

            extension = self._extensions[name](*self.args, **config)
            extension.extendMarkdown(md)

# I have no idea why it's not calling man_pages' makeExtension, but oh well.
def makeExtension(*args, **kwargs):
    return NiHExtension(*args, **kwargs)
