# Downloads

### Latest Release

The latest release of CARLA is **0.9.16**, check out the [release announcement](https://carla.org/2025/09/16/release-0.9.16/) for more details.

- [CARLA 0.9.16](https://github.com/carla-simulator/carla/releases/tag/0.9.16/) - [Documentation](https://carla.readthedocs.io/en/latest/)

### Nightly build

This is an automated build with the latest changes pushed to our `ue4-dev`
branch. It contains the very latest fixes and features that will be part of the
next release, but also some experimental changes. Use at your own risk!

- [CARLA Nightly Build (Linux)](https://carla-releases.s3.us-east-005.backblazeb2.com/Linux/Dev/CARLA_Latest.tar.gz)
- [AdditionalMaps Nightly Build (Linux)](https://carla-releases.s3.us-east-005.backblazeb2.com/Linux/Dev/AdditionalMaps_Latest.tar.gz)
- [CARLA Nightly Build (Windows)](https://carla-releases.s3.us-east-005.backblazeb2.com/Windows/Dev/CARLA_Latest.zip)
- [AdditionalMaps Nightly Build (Windows)](https://carla-releases.s3.us-east-005.backblazeb2.com/Windows/Dev/AdditionalMaps_Latest.zip)

<p><a id="last-run-link" href='https://github.com/carla-simulator/carla/actions'>Last successful build</a>: <span id="last-run-time" class="loading">Loading...</span></p>

!!! note
    If you are looking for the nightly build of the Unreal Engine 5.5 version of CARLA, please visit the [downloads page](https://carla-ue5.readthedocs.io/en/latest/download/) of the [CARLA UE5 documentation](https://carla-ue5.readthedocs.io/en/latest/).

### Versions 0.9.x

Here are the previous versions of CARLA with links to the specific documentation for each version:

- [CARLA 0.9.15](https://github.com/carla-simulator/carla/releases/tag/0.9.15/) - [Documentation](https://carla.readthedocs.io/en/0.9.15/)
- [CARLA 0.9.14](https://github.com/carla-simulator/carla/releases/tag/0.9.14/) - [Documentation](https://carla.readthedocs.io/en/0.9.14/)
- [CARLA 0.9.13](https://github.com/carla-simulator/carla/releases/tag/0.9.13/) - [Documentation](https://carla.readthedocs.io/en/0.9.13/)
- [CARLA 0.9.12](https://github.com/carla-simulator/carla/releases/tag/0.9.12/) - [Documentation](https://carla.readthedocs.io/en/0.9.12/)
- [CARLA 0.9.11](https://github.com/carla-simulator/carla/releases/tag/0.9.11/) - [Documentation](https://carla.readthedocs.io/en/0.9.11/)
- [CARLA 0.9.10](https://github.com/carla-simulator/carla/releases/tag/0.9.10/) - [Documentation](https://carla.readthedocs.io/en/0.9.10/)
- [CARLA 0.9.9](https://github.com/carla-simulator/carla/releases/tag/0.9.9/) - [Documentation](https://carla.readthedocs.io/en/0.9.9/)
- [CARLA 0.9.8](https://github.com/carla-simulator/carla/releases/tag/0.9.8/) - [Documentation](https://carla.readthedocs.io/en/0.9.8/)
- [CARLA 0.9.7](https://github.com/carla-simulator/carla/releases/tag/0.9.7/) - [Documentation](https://carla.readthedocs.io/en/0.9.7/)
- [CARLA 0.9.6](https://github.com/carla-simulator/carla/releases/tag/0.9.6/) - [Documentation](https://carla.readthedocs.io/en/0.9.6/)
- [CARLA 0.9.5](https://github.com/carla-simulator/carla/releases/tag/0.9.5/) - [Documentation](https://carla.readthedocs.io/en/0.9.5/)
- [CARLA 0.9.4](https://github.com/carla-simulator/carla/releases/tag/0.9.4/) - [Documentation](https://carla.readthedocs.io/en/0.9.4/)
- [CARLA 0.9.3](https://github.com/carla-simulator/carla/releases/tag/0.9.3/) - [Documentation](https://carla.readthedocs.io/en/0.9.3/)
- [CARLA 0.9.2](https://github.com/carla-simulator/carla/releases/tag/0.9.2/) - [Documentation](https://carla.readthedocs.io/en/0.9.2/)
- [CARLA 0.9.1](https://github.com/carla-simulator/carla/releases/tag/0.9.1/) - [Documentation](https://carla.readthedocs.io/en/0.9.1/)
- [CARLA 0.9.0](https://github.com/carla-simulator/carla/releases/tag/0.9.0/) - [Documentation](https://carla.readthedocs.io/en/0.9.0/)

### Versions 0.8.x

- [CARLA 0.8.4](https://github.com/carla-simulator/carla/releases/tag/0.8.4/) - [Documentation](https://carla.readthedocs.io/en/0.8.4/)
- [CARLA 0.8.3](https://github.com/carla-simulator/carla/releases/tag/0.8.3/)
- [CARLA 0.8.2](https://github.com/carla-simulator/carla/releases/tag/0.8.2/) - [Documentation](https://carla.readthedocs.io/en/stable/)

- - -

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
getLastWorkflowRun('carla-simulator', 'carla', 'ue4_dev.yml')
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