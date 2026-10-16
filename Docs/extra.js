$(document).ready(function () {
    if(window.location.href.indexOf("/latest/") != -1) {
        var banner = document.createElement("div");
        var section = document.getElementsByClassName("section");
        banner.className = "dev-version-warning";
        banner.innerHTML = "You are currently reading documentation for the \"ue58-dev\" branch of the Unreal Engine 5 version of CARLA. The functionality referred to in this documentation may exhibit instability or unexpected behavior.";
        section[0].insertBefore(banner,section[0].childNodes[0]);
    }

});

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
