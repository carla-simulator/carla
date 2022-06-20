# 国际化

由于 Docute 使用基于 URL 的 API实现，因此添加多语言支持非常简单：

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

使用上述文件夹结构，用户可以通过 URL `/zh/` 访问文档的*中文*版本。

然后，可以使用 `overrides` 选项来本地化 UI 中使用的文本：

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
