# Internationalization

As Docute uses a URL-based API, adding multi-language support can be pretty easy:

```
docs
├─ README.md
├─ foo.md
├─ nested
│  └─ README.md
└─ zh
   ├─ README.md
   ├─ foo.md
   └─ nested
      └─ README.md
```

With above folder structure, users can visit the *Chinese* version of your docs via URL `/zh/`.

Then you can use the `overrides` option to localize the text used in UI:

```js
new Docute({
  sidebar: [
    {
      children: [
        { title: 'Guide', link: '/guide' }
      ]
    }
  ],
  overrides: {
    '/': {
      language: 'English' // Used by the language dropdown menu in the sidebar
    },
    '/zh/': {
      language: 'Chinese',
      // Override the default sidebar
      sidebar: [
        {
          children: [
            { title: '指南', link: '/zh/guide' }
          ]
        }
      ]
    }
  }
})
```
