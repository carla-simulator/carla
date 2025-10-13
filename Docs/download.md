# Download

### Latest Release

- [CARLA 0.10.0](https://github.com/carla-simulator/carla/releases/tag/0.10.0) - [Documentation](https://carla-ue5.readthedocs.io/en/latest/)

### Nightly build

This is an automated build with the latest changes pushed to our `ue5-dev`
branch. It contains the very latest fixes and features that will be part of the
next release, but also some experimental changes. Use at your own risk!

- [CARLA Nightly Build (Linux)](https://s3.us-east-005.backblazeb2.com/carla-releases/Linux/Dev/CARLA_UE5_Latest.tar.gz)

<p><a id="last-run-link" href='https://github.com/carla-simulator/carla/actions'>Last successful build</a>: <span id="last-run-time" class="loading">Loading...</span></p>

!!! note
    If you are looking for the nightly build of the Unreal Engine 4.26 version of CARLA, please visit the [downloads page](https://carla.readthedocs.io/en/latest/download/) of the [CARLA UE4 documentation](https://carla.readthedocs.io/en/latest/).

### Docker

All versions are available to pull from DockerHub:

```sh
docker pull carlasim/carla:X.X.X
```

Use tag "latest" for the most recent release:

```sh
docker pull carlasim/carla:latest
```

<script>
async function getLastWorkflowRun(owner, repo, workflowFileName) {
  const url = `https://api.github.com/repos/${owner}/${repo}/actions/workflows/${workflowFileName}/runs?status=completed&per_page=1`;
  
  try {
    const response = await fetch(url, {
      headers: {
        'Accept': 'application/vnd.github.v3+json'
      }
    });
    
    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }
    
    const data = await response.json();
    if (data.workflow_runs && data.workflow_runs.length > 0) {
      const lastRun = data.workflow_runs[0];
      return {
        timestamp: lastRun.updated_at,
        url: lastRun.html_url,
        status: lastRun.conclusion
      };
    }
    return null;
  } catch (error) {
    console.error('Error fetching workflow runs:', error);
    return null;
  }
}

// Format timestamp to be more readable
function formatTimestamp(isoString) {
    const date = new Date(isoString);
    return date.toLocaleString('en-US', {
        weekday: 'long',
        year: 'numeric',
        month: 'long',
        day: 'numeric',
        hour: '2-digit',
        minute: '2-digit',
        timeZoneName: 'short'
    });
}

// Example usage
getLastWorkflowRun('carla-simulator', 'carla', 'ue5_dev.yml')
  .then(result => {
    if (result) {
      console.log('Last successful run:', result.timestamp);
      console.log('View run:', result.url);
      const lastRunTimeElement = document.getElementById('last-run-time');
      const lastRunLink = document.getElementById('last-run-link')
      //const lastRun = result.workflow_runs[0];
      const formattedTime = formatTimestamp(result.timestamp);
      lastRunTimeElement.textContent = formattedTime;
      lastRunLink.setAttribute("href", result.url)

    } else {
      console.log('No completed runs found');
    }
  });
</script>
