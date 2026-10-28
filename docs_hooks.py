"""mkdocs hooks: page-level HTML touch-ups that would otherwise mean editing
every page. Registered in mkdocs.yml under `hooks`."""

import re

# Any <img> that does not already say how to load itself. Browsers fetch eagerly
# by default, so a catalogue page pulls every image before the reader scrolls.
_IMG_WITHOUT_LOADING = re.compile(r'<img (?![^>]*\bloading=)')


def on_post_page(output, page, config):
    return _IMG_WITHOUT_LOADING.sub('<img loading="lazy" ', output)
