# Customization

Customizing Docute is as fun as playing with Lego bricks.

## Navbar

The navbar is used for site-level navigation. It usually contains a link to your homepage and a link to your project's repository. However you can add whatever you want there.

```js
new Docute({
  title: 'Docute',
  nav: [
    {
      title: 'Home',
      link: '/'
    },
    {
      title: 'GitHub',
      link: 'https://github.com/egoist/docute'
    },
    // A dropdown menu
    {
      title: 'Community',
      children: [
        {
          title: 'Spectrum',
          link: 'https://spectrum.chat/your-community'
        },
        {
          title: 'Discord',
          link: 'https://discord.app/your-discord-server'
        }
      ]
    }
  ]
})
```

The `title` option defaults to the value of `<title>` tag in your HTML, so it's completely optional.

Check out the navbar of this website to see how it looks.

## Sidebar

Sidebar is mainly used for navigations between pages. As you can see from this page, we also use it to display a version selector and a language selector.

```js
new Docute({
  sidebar: [
    // A sidebar item, with child links
    {
      title: 'Guide',
      children: [
        {
          title: 'Getting Started',
          link: '/guide/getting-started'
        },
        {
          title: 'Installation',
          link: '/guide/installation'
        }
      ]
    },
    // An external link
    {
      title: 'GitHub',
      link: 'https://github.com/egoist/docute'
    }
  ]
})
```

Check out the [sidebar](../options.md#sidebar) option reference for more details.

## Layout

Docute by default uses a wide-screen layout as you see, but there're more layouts available:

<docute-select v-model="$store.state.originalConfig.layout" v-slot="{ value }">
  <option value="wide" :selected="value === 'wide'">Wide</option>
  <option value="narrow" :selected="value === 'narrow'">Narrow</option>
  <option value="left" :selected="value === 'left'">Left</option>
</docute-select>

```js {interpolate:true}
new Docute({
  layout: '{{ $store.state.originalConfig.layout }}'
})
```

## Versioning

Let's say you have `master` branch for the latest docs and `v0.1` `v0.2` branches for older versions, you can use one Docute website to serve them all, with the help of [`overrides`](../options.md#overrides) and [`sourcePath`](../options.md#sourcepath) option.

```js
new Docute({
  // Configure following paths to load Markdown files from different path
  overrides: {
    '/v0.1/': {
      sourcePath: 'https://raw.githubusercontent.com/user/repo/v0.1'
    },
    '/v0.2/': {
      sourcePath: 'https://raw.githubusercontent.com/user/repo/v0.2'
    }
  },
  // Use `versions` option to add a version selector
  // In the sidebar
  versions: {
    'v1 (Latest)': {
      link: '/'
    },
    'v0.2': {
      link: '/v0.2/'
    },
    'v0.1': {
      link: '/v0.1/'
    }
  }
})
```

## Custom Fonts

Apply custom fonts to your website is pretty easy, you can simply add a `<style>` tag in your HTML file to use [Google Fonts](https://fonts.google.com/):

```html
<style>
  /* Import desired font from Google fonts */
  @import url('https://fonts.googleapis.com/css?family=Lato');

  /* Apply the font to body (to override the default one) */
  body {
    font-family: Lato, sans-serif;
  }
</style>
```

<button @click="insertCustomFontsCSS">Click me</button> to toggle the custom fonts on this website.

By default a fresh Docute website will use system default fonts.

## Custom Style

You can use [`cssVariables`](../options.md#cssvariables) option to customize site style:

```js
new Docute({
  cssVariables: {
    sidebarWidth: '300px'
  }
})

// Or using a function to get current theme
new Docute({
  cssVariables(theme) {
    return theme === 'dark' ? {} : {}
  }
})
```

The `cssVariables` used by the the <code>{{ $store.getters.config.theme }}</code> theme:

<ul>
<li v-for="(value, key) in $store.getters.cssVariables" :key="key">
<strong>{{key}}</strong>: <color-box :color="value" v-if="/(Color|Background)/.test(key)" />
<code>{{value}}</code>
</li>
</ul>

Note that these properties are defined in camelCase but you should reference them in CSS using kebab-case:

```css
.Sidebar {
  width: var(--sidebar-width);
}
```
