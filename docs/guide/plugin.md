# Plugin

A plugin is essentially a pure object:

```js
const showAuthor = {
  // Plugin name
  name: 'showAuthor',
  // Extend core features
  extend(api) {
    api.processMarkdown(text => {
      return text.replace(/{author}/g, '> Written by EGOIST')
    })
  }
}

new Docute({
  // ...
  plugins: [
    showAuthor
  ]
})
```

Example:

```markdown
# Page Title

{author}
```

<ImageZoom :border="true" src="https://i.loli.net/2018/09/28/5bae278dd9c03.png" />

---

To accept options in your plugin, you can use a factory function:

```js
const myPlugin = opts => {
  return {
    name: 'my-plugin',
    extend(api) {
      // do something with `opts` and `api`
    }
  }
}

new Docute({
  plugins: [
    myPlugin({ foo: true })
  ]
})
```

---

For more information on how to develop a plugin, please check out [Plugin API](/plugin-api).

Check out [https://github.com/egoist/docute-plugins](https://github.com/egoist/docute-plugins) for a list of Docute plugins by the maintainers and users.
