// Download page: show when the nightly was last built successfully.
async function getLastWorkflowRun(owner, repo, workflowFileName) {
    const url = `https://api.github.com/repos/${owner}/${repo}/actions/workflows/${workflowFileName}/runs?status=success&per_page=1`;
    try {
        const response = await fetch(url, { headers: { 'Accept': 'application/vnd.github.v3+json' } });
        if (!response.ok) { return null; }
        const data = await response.json();
        if (data.workflow_runs && data.workflow_runs.length > 0) {
            const lastRun = data.workflow_runs[0];
            return { timestamp: lastRun.updated_at, url: lastRun.html_url };
        }
    } catch (error) {
        // The GitHub API is rate limited per IP; failing quietly is fine here.
    }
    return null;
}

$(document).ready(function () {
    const timeElement = document.getElementById('last-run-time');
    const linkElement = document.getElementById('last-run-link');
    if (!timeElement) { return; }
    getLastWorkflowRun('carla-simulator', 'carla', 'ue5_dev.yml').then(result => {
        if (!result) { timeElement.textContent = 'unavailable'; return; }
        timeElement.textContent = new Date(result.timestamp).toLocaleString('en-US', {
            weekday: 'long', year: 'numeric', month: 'long', day: 'numeric',
            hour: '2-digit', minute: '2-digit', timeZoneName: 'short'
        });
        if (linkElement) { linkElement.setAttribute('href', result.url); }
    });
});

// Copy button on every code block: the readthedocs theme has none and the
// markdown pipeline emits a plain <pre><code>, so it is added from the DOM.
function carlaCopyText(text) {
    if (navigator.clipboard && window.isSecureContext) {
        return navigator.clipboard.writeText(text).then(function () { return true; },
                                                        function () { return false; });
    }
    // file:// and plain http: have no clipboard API; fall back to a textarea.
    var area = document.createElement("textarea");
    area.value = text;
    area.style.position = "fixed";
    area.style.opacity = "0";
    document.body.appendChild(area);
    area.select();
    var ok = false;
    try { ok = document.execCommand("copy"); } catch (e) { ok = false; }
    document.body.removeChild(area);
    return Promise.resolve(ok);
}

$(document).ready(function () {
    var blocks = document.querySelectorAll(".rst-content pre > code");
    Array.prototype.forEach.call(blocks, function (code) {
        // The API snippets carry their own copy button.
        if (code.closest(".SnipetContent")) { return; }
        var pre = code.parentNode;
        var wrapper = document.createElement("div");
        wrapper.className = "code-block";
        pre.parentNode.insertBefore(wrapper, pre);
        wrapper.appendChild(pre);

        // FontAwesome 4.7 ships with the theme, so the glyph costs nothing.
        var button = document.createElement("button");
        button.type = "button";
        button.className = "code-copy";
        button.title = "Copy to clipboard";
        button.setAttribute("aria-label", "Copy to clipboard");
        var icon = document.createElement("span");
        icon.className = "fa fa-clipboard";
        button.appendChild(icon);
        button.addEventListener("click", function () {
            carlaCopyText(code.innerText).then(function (ok) {
                icon.className = ok ? "fa fa-check" : "fa fa-times";
                setTimeout(function () { icon.className = "fa fa-clipboard"; }, 1500);
            });
        });
        wrapper.appendChild(button);
    });
});

// Click an image to see it full size: the pages are ~800px wide while the
// images are up to 1920px, so the detail is there but never shown. Once open,
// the mouse wheel, the arrow keys, the side buttons or a swipe move through
// every image of the page.
$(document).ready(function () {
    var overlay = null, big = null, caption = null, counter = null;
    var gallery = [], current = 0, wheelLock = 0, touchX = null;

    // Catalogue pages load <name>_thumb.webp; the full shot sits beside it
    // under the same name.
    function fullSize(image) { return image.src.replace(/_thumb\.webp$/, ".webp"); }

    // The caption is the .img-caption block right after the image's paragraph.
    function captionOf(image) {
        var block = image.closest("p") || image;
        var next = block.nextElementSibling;
        if (next && next.classList.contains("img-caption")) { return next.textContent.trim(); }
        return image.alt || "";
    }

    function closeOverlay() {
        if (overlay) { overlay.remove(); overlay = null; }
    }

    function show(index) {
        current = (index + gallery.length) % gallery.length;
        var image = gallery[current];
        big.src = fullSize(image);
        big.alt = image.alt || "";
        caption.textContent = captionOf(image);
        counter.textContent = gallery.length > 1 ? (current + 1) + " / " + gallery.length : "";
    }

    function button(cls, label, step) {
        var b = document.createElement("button");
        b.type = "button";
        b.className = "img-lightbox-nav " + cls;
        b.setAttribute("aria-label", label);
        b.innerHTML = step < 0 ? "&#8249;" : "&#8250;";
        b.addEventListener("click", function (event) { event.stopPropagation(); show(current + step); });
        return b;
    }

    function openOverlay(index) {
        closeOverlay();
        overlay = document.createElement("div");
        overlay.className = "img-lightbox";
        big = document.createElement("img");
        caption = document.createElement("div");
        caption.className = "img-lightbox-caption";
        counter = document.createElement("div");
        counter.className = "img-lightbox-counter";
        overlay.appendChild(big);
        overlay.appendChild(caption);
        overlay.appendChild(counter);
        if (gallery.length > 1) {
            overlay.appendChild(button("prev", "Previous image", -1));
            overlay.appendChild(button("next", "Next image", 1));
        }
        overlay.addEventListener("click", closeOverlay);
        overlay.addEventListener("wheel", function (event) {
            event.preventDefault();
            // One trackpad gesture fires dozens of wheel events: one step per gesture.
            var now = Date.now();
            if (now - wheelLock < 350 || Math.abs(event.deltaY) < 4) { return; }
            wheelLock = now;
            show(current + (event.deltaY > 0 ? 1 : -1));
        }, { passive: false });
        overlay.addEventListener("touchstart", function (event) { touchX = event.touches[0].clientX; }, { passive: true });
        overlay.addEventListener("touchend", function (event) {
            if (touchX === null) { return; }
            var dx = event.changedTouches[0].clientX - touchX;
            touchX = null;
            if (Math.abs(dx) > 50) { event.preventDefault(); show(current + (dx < 0 ? 1 : -1)); }
        });
        document.body.appendChild(overlay);
        show(index);
    }

    document.addEventListener("keydown", function (event) {
        if (!overlay) { return; }
        if (event.key === "Escape") { closeOverlay(); }
        else if (event.key === "ArrowRight" || event.key === "ArrowDown") { event.preventDefault(); show(current + 1); }
        else if (event.key === "ArrowLeft" || event.key === "ArrowUp") { event.preventDefault(); show(current - 1); }
    });

    var images = document.querySelectorAll(".rst-content img");
    Array.prototype.forEach.call(images, function (image) {
        // Leave the badges and anything that is already a link alone.
        if (image.closest("a") || image.src.indexOf("/img/btn/") !== -1) { return; }
        image.classList.add("zoomable");
        var index = gallery.length;
        gallery.push(image);
        image.addEventListener("click", function () { openOverlay(index); });
    });
});


/*************************** JUSTIFIED PROSE ****************************/

// Justifying a line that ends just before a long function name stretches its
// spaces; measure first and justify only the blocks that come out tight.
document.addEventListener("DOMContentLoaded", function () {
    var MAX_SLACK = 0.12;
    var skip = "table, .build-buttons, .vector-zero, .img-caption, .admonition-title";

    var blocks = Array.prototype.filter.call(
        document.querySelectorAll(".rst-content p, .rst-content li"),
        function (node) {
            return !node.closest(skip) && !node.querySelector("img") &&
                node.textContent.trim().length > 0;
        });

    // A range reports one rect per inline box, so merge the rects that share a
    // top edge back into the line they belong to.
    function lineWidths(node) {
        var range = document.createRange();
        range.selectNodeContents(node);
        var rects = range.getClientRects();
        var lines = {};
        for (var i = 0; i < rects.length; i++) {
            var rect = rects[i];
            if (!rect.width && !rect.height) { continue; }
            var key = Math.round(rect.top);
            var line = lines[key];
            if (!line) { lines[key] = { left: rect.left, right: rect.right }; continue; }
            line.left = Math.min(line.left, rect.left);
            line.right = Math.max(line.right, rect.right);
        }
        return Object.keys(lines).sort(function (a, b) { return a - b; })
            .map(function (key) { return lines[key].right - lines[key].left; });
    }

    function justify() {
        blocks.forEach(function (node) { node.classList.remove("justified"); });

        var keep = blocks.filter(function (node) {
            var lines = lineWidths(node);
            if (lines.length < 2) { return false; }
            var full = Math.max.apply(null, lines);
            // The last line is never stretched, so it does not count.
            for (var i = 0; i < lines.length - 1; i++) {
                if ((full - lines[i]) / full > MAX_SLACK) { return false; }
            }
            return true;
        });

        keep.forEach(function (node) { node.classList.add("justified"); });
    }

    var pending;
    window.addEventListener("resize", function () {
        clearTimeout(pending);
        pending = setTimeout(justify, 150);
    });

    justify();
    window.addEventListener("load", justify);
});
