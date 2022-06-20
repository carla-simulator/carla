# Docute

一种轻松创建文档的方式。

## 什么是 Docute ？

Docute 本质上就是一个 JavaScript 文件，它可以获取 Markdown 文件并将它们呈现为单页面应用。

它完全由运行时驱动，因此并不涉及服务端组件，这就意味着没有构建过程。你只需创建一个 HTML 文件和一堆 Markdown 文档，你的网站就差不多完成了！

## 它如何工作？

简而言之：URL 是 API。

访问你的 URL 时，会获取并呈现相应的 markdown 文件：

```
/         => /README.md
/foo      => /foo.md
/foo/     => /foo/README.md
/foo/bar  => /foo/bar.md
```

## 快速开始

假设你在 `./my-docs` 文件夹中有以下文件：

```bash
.
├── README.md
└── index.html
```

`index.html` 看起来像这样：

```html {highlight:[7,'10-16']}
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
    <title>My Docs</title>
    <link rel="stylesheet" href="https://unpkg.com/docute@4/dist/docute.css">
  </head>
  <body>
    <div id="docute"></div>
    <script src="https://unpkg.com/docute@4/dist/docute.js"></script>
    <script>
      new Docute({
        target: '#docute'
      })
    </script>
  </body>
</html>
```

然后你可以使用以下命令将此文件夹作为计算机上的静态网站展示：

- Node.js: `npm i -g serve && serve .`
- Python: `python -m SimpleHTTPServer`
- Golang: `caddy`
- ..或任何静态 web 服务器

下一步, 你可能会想用 [sidebar](./options.md#sidebar), [nav](./options.md#nav) 或其他 [选项](./options.md) 来定制你的文档。 

这里有一个在线演示，你可以[立刻预览](https://repl.it/@egoist/docute-starter)或者 [下载](https://repl.it/@egoist/docute-starter.zip) 到本地运行。

## 比较

### VuePress / GitBook / Hexo

这些项目在构建时会生成静态的 HTML，这对 SEO 非常有帮助。

如果你在意 SEO，那你可能会喜欢使用 [presite](https://github.com/egoist/presite) 来预渲染你的网站。

### Docsify

[Docsify](https://docsify.js.org/#/) 和 Docute 几乎相同，但具有不同的 UI 和不同的使用方式。

Docute（60kB）比 Docisfy（20kB）大 3 倍，因为我们使用了 Vue，Vue Router 和 Vuex，而 Docsify 使用的是 vanilla JavaScript。


## 浏览器兼容性

Docute 支持所有常青浏览器（ever-green browsers），即没有对 IE 进行支持！
