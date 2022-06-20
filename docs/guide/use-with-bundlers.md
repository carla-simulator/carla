# Use With Bundlers

You don't have to use a bundler in order to use Docute, however if you want, you can!

First you need to install Docute as a dependency in your project:

```bash
npm install docute
```

Then see below for the usage with your bundler of choice.

## Webpack

In your entry file:

```js
import Docute from 'docute'

new Docute({
  target: '#app',
  // Other options
})
```

You may need to use [html-webpack-plugin](https://github.com/jantimon/html-webpack-plugin) to generate an HTML file too.

If you're using [Vue CLI](https://cli.vuejs.org) or [Poi](https://poi.js.org), congratulations, there's no more build configs needed.

## Parcel

[TODO] [PR WELCOME]
