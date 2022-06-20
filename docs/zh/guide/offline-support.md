# Offline Support

Improve your website's performance by caching and serving your files, powered by a [service worker](https://developer.mozilla.org/docs/Web/API/Service_Worker_API/Using_Service_Workers).

First create a `sw.js` in your docs root directory:

```js
importScripts(
  'https://storage.googleapis.com/workbox-cdn/releases/3.6.1/workbox-sw.js'
)

const ALLOWED_HOSTS = [
  // The domain to load markdown files
  location.host,
  // The domain to load docute
  'unpkg.com'
]

const matchCb = ({ url, event }) => {
  return event.request.method === 'GET' && ALLOWED_HOSTS.includes(url.host)
}

workbox.routing.registerRoute(
  matchCb,
  workbox.strategies.networkFirst()
)
```

<sup>_[Workbox](https://developers.google.com/web/tools/workbox/) is a library that bakes in a set of best practices and removes the boilerplate every developer writes when working with service workers._</sup>

Then register this service worker in `index.html`:

```html {highlight:['16-18']}
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8" />
    <meta
      name="viewport"
      content="width=device-width, initial-scale=1, shrink-to-fit=no"
    />
    <title>My Docs</title>
    <link rel="stylesheet" href="https://unpkg.com/docute@4/dist/docute.css" />
  </head>
  <body>
    <div id="docute"></div>
    <script src="https://unpkg.com/docute@4/dist/docute.js"></script>
    <script>
      if ('serviceWorker' in navigator) {
        navigator.serviceWorker.register('/sw.js')
      }

      new Docute({
        target: '#docute'
      })
    </script>
  </body>
</html>
```

__🥳 Now your website will be offline-ready.__

If you somehow no longer need this service worker, replace the content of `sw.js` with following code to disable it:

```js
self.addEventListener('install', e => {
  self.skipWaiting()
})

self.addEventListener('activate', e => {
  self.registration
    .unregister()
    .then(() => {
      return self.clients.matchAll()
    })
    .then(clients => {
      clients.forEach(client => client.navigate(client.url))
    })
})
```
