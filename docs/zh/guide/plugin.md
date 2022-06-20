# 插件

插件本质上是一个纯对象（pure object）：

```js
const showAuthor = {
  // 插件名称
  name: 'showAuthor',
  // 扩展核心功能
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

示例:

```markdown
# Page 标题

{author}
```

<ImageZoom :border="true" src="https://i.loli.net/2018/09/28/5bae278dd9c03.png" />

---

要接收插件中的选项，可以使用工厂函数：

```js
const myPlugin = opts => {
  return {
    name: 'my-plugin',
    extend(api) {
      // 使用 `opts` 和 `api` 做点什么
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

欲了解更多如何开发插件的信息，请查阅[插件 API](/zh/plugin-api)。
