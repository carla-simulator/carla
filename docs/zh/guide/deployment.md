# 部署

请记住，它只是一个在任何地方都可以提供的静态 HTML 文件。

## ZEIT Now <Badge type="success">Recommended</Badge>

[ZEIT Now](https://zeit.co/now) is a platform for Global Serverless Deployments, it's also perfectly suitable for deploying a static website with or without build process.

Assuming you have your docs in `./docs` folder, to deploy it you can simply populate a `now.json` in your project:

```json
{
  "version": 2,
  "builds": [
    {
      "src": "docs/**",
      "use": "@now/static"
    }
  ]
}
```

Then [install Now](https://zeit.co/docs/v2/getting-started/installation/) on your machine.

After that, you can run the command `now` in your project and you're all set.

Make sure to check out Now's [GitHub Integration](https://zeit.co/docs/v2/integrations/now-for-github/) if you want automatic deployments on every push and pull request.

## Netlify <Badge type="success">Recommended</Badge>

1. 登录你的 [Netlify](https://www.netlify.com/) 账号。
2. 在 [dashboard](https://app.netlify.com/) 页，点击 __New site from Git__.
3. 选择一个存储文档的仓库，将 __Build Command__ 区域留空， blank，用 `index.html` 的目录填写 __Publish directory__ 区域，例如，`docs/index.html`，应填为 `docs`。

## GitHub Pages

使用 Github Pages 最简单的方式是在 master 分支上的 `./docs` 文件夹中加入所有文件，然后将此文件夹用于 Github Pages：

<ImageZoom src="https://i.loli.net/2018/06/11/5b1e0da0c173a.png" alt="github pages" :border="true" />

但是你仍然可以使用 `gh-pages` 分支或者 `master` 分支来为你的文档提供服务，这一切都取决于你的需求。
