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
