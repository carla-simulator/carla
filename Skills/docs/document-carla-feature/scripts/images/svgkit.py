"""Tiny SVG helpers shared by the doc diagrams (consistent palette and fonts)."""
import html
FONT = "Lato, 'Helvetica Neue', Arial, sans-serif"
MONO = "'DejaVu Sans Mono', Consolas, monospace"
C = dict(ink="#2b2b2b", sub="#5f6b73", line="#c9d3da", card="#f5f8fa", blue="#2980b9", red="#d9534f",
         green="#3c9a5f", orange="#e69138", purple="#7e57c2", white="#ffffff", soft_blue="#e3f0f9",
         soft_red="#fbe6e5", soft_green="#e4f3e9", soft_orange="#fdf0e1", soft_purple="#efe9f8")


def esc(s):
    return html.escape(str(s))


class Svg:
    def __init__(self, w, h, bg=C["white"]):
        self.w, self.h, self.parts = w, h, []
        self.parts.append(f'<rect width="{w}" height="{h}" fill="{bg}"/>')

    def add(self, s):
        self.parts.append(s)

    def text(self, x, y, s, size=16, color=C["ink"], weight="normal", anchor="start", mono=False, italic=False):
        fam = MONO if mono else FONT
        st = ' font-style="italic"' if italic else ''
        self.add(f'<text x="{x}" y="{y}" font-family="{fam}" font-size="{size}" fill="{color}" '
                 f'font-weight="{weight}" text-anchor="{anchor}"{st} xml:space="preserve">{esc(s)}</text>')

    def rect(self, x, y, w, h, fill=C["card"], stroke=C["line"], r=10, sw=1.5, dash=None):
        d = f' stroke-dasharray="{dash}"' if dash else ''
        self.add(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{r}" fill="{fill}" stroke="{stroke}" stroke-width="{sw}"{d}/>')

    def line(self, x1, y1, x2, y2, color=C["ink"], sw=2, arrow=None, dash=None):
        m = f' marker-end="url(#ah_{arrow})"' if arrow else ''
        d = f' stroke-dasharray="{dash}"' if dash else ''
        self.add(f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="{color}" stroke-width="{sw}"{m}{d}/>')

    def path(self, d, color=C["ink"], sw=2, fill="none", arrow=None, dash=None):
        m = f' marker-end="url(#ah_{arrow})"' if arrow else ''
        da = f' stroke-dasharray="{dash}"' if dash else ''
        self.add(f'<path d="{d}" stroke="{color}" stroke-width="{sw}" fill="{fill}"{m}{da}/>')

    def circle(self, x, y, r, fill="none", stroke=C["ink"], sw=2):
        self.add(f'<circle cx="{x}" cy="{y}" r="{r}" fill="{fill}" stroke="{stroke}" stroke-width="{sw}"/>')

    def box(self, x, y, w, h, title, lines=(), fill=C["soft_blue"], stroke=C["blue"], tsize=17, lsize=13.5, mono_lines=False):
        self.rect(x, y, w, h, fill=fill, stroke=stroke, sw=2)
        n = len(lines)
        ty = y + h / 2 + 6 - (n * (lsize + 5)) / 2
        self.text(x + w / 2, ty, title, size=tsize, weight="bold", anchor="middle")
        for i, l in enumerate(lines):
            self.text(x + w / 2, ty + (i + 1) * (lsize + 6), l, size=lsize, color=C["sub"], anchor="middle", mono=mono_lines)

    def save(self, path):
        cols = {k: v for k, v in C.items()}
        markers = ''.join(
            f'<marker id="ah_{k}" viewBox="0 0 10 10" refX="8.5" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">'
            f'<path d="M0,0 L10,5 L0,10 z" fill="{v}"/></marker>' for k, v in cols.items())
        body = '\n'.join(self.parts)
        open(path, 'w').write(
            f'<svg xmlns="http://www.w3.org/2000/svg" width="{self.w}" height="{self.h}" viewBox="0 0 {self.w} {self.h}">'
            f'<defs>{markers}</defs>\n{body}\n</svg>\n')


def render_png(svg_path, png_path, w, h):
    """Rasterise with headless Chrome (preview only)."""
    import subprocess, os
    html_path = png_path + '.html'
    open(html_path, 'w').write(f'<html><body style="margin:0"><img src="file://{os.path.abspath(svg_path)}"></body></html>')
    subprocess.run(['google-chrome', '--headless=new', '--disable-gpu', '--hide-scrollbars', f'--window-size={w},{h + 400}',
                    f'--screenshot={png_path}', 'file://' + os.path.abspath(html_path)],
                   capture_output=True, timeout=60)
    os.remove(html_path)
    from PIL import Image
    Image.open(png_path).crop((0, 0, w, h)).save(png_path)
