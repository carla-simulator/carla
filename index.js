import html from 'html-template-tag'
import googleAnalytics from 'docute-google-analytics'
import Docute from '../src'
import prismLanguages from '../src/utils/prismLanguages'
import ColorBox from './components/ColorBox.vue'

const SponsorIcon = {
  template: html`
  <svg style="color:#ea4aaa;" fill="currentColor" viewBox="0 0 12 16" version="1.1" width="12" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M8.727 3C7.091 3 6.001 4.65 6.001 4.65S4.909 3 3.273 3C1.636 3 0 4.1 0 6.3 0 9.6 6 14 6 14s6-4.4 6-7.7C12 4.1 10.364 3 8.727 3z"></path></svg>
  `
}

new Docute({
  target: 'app',
  title: 'Docute',
  highlight: ['typescript', 'bash', 'json', 'markdown'],
  plugins: [
    process.env.NODE_ENV === 'production' && googleAnalytics('UA-54857209-11')
  ].filter(Boolean),
  editLinkBase: 'https://github.com/egoist/docute/tree/master/website/docs',
  editLinkText: 'Edit this page on GitHub',
  router: {
    mode: 'history'
  },
  detectSystemDarkTheme: true,
  darkThemeToggler: true,
  sourcePath: '/',
  componentMixins: [
    {
      data() {
        return {
          builtinLanguages: prismLanguages.builtin,
          deps: __DEPS__
        }
      },
      methods: {
        insertCustomFontsCSS() {
          const ID = 'custom-fonts-css'
          const existing = document.getElementById(ID)
          if (existing) {
            existing.parentNode.removeChild(existing)
          } else {
            const style = document.createElement('style')
            style.id = ID
            style.textContent = `
            /* Import desired font from Google fonts */
            @import url('https://fonts.googleapis.com/css?family=Lato');

            /* Apply the font to body (to override the default one) */
            body {
              font-family: Lato, sans-serif;
            }
            `
            document.head.appendChild(style)
          }
        }
      },
      components: {
        ColorBox
      }
    }
  ],
  versions: {
    'v4 (Latest)': {
      link: '/'
    },
    v3: {
      link: 'https://docute3.egoist.sh'
    }
  },
  nav: [
    {
      title: 'Home',
      link: '/'
    },
    {
      title: 'GitHub',
      link: 'https://github.com/egoist/docute'
    }
  ],
  sidebar: [
    {
      title: 'Guide',
      children: [
        {
          title: 'Introduction',
          link: '/'
        },
        {
          title: 'Customization',
          link: '/guide/customization'
        },
        {
          title: 'Markdown Features',
          link: '/guide/markdown-features'
        },
        {
          title: 'Use Vue in Markdown',
          link: '/guide/use-vue-in-markdown'
        },
        {
          title: 'Internationalization',
          link: '/guide/internationalization'
        },
        {
          title: 'Plugin',
          link: '/guide/plugin'
        },
        {
          title: 'Deployment',
          link: '/guide/deployment'
        }
      ]
    },
    {
      title: 'Advanced',
      children: [
        {
          title: 'Use With Bundlers',
          link: '/guide/use-with-bundlers'
        },
        {
          title: 'Offline Support',
          link: '/guide/offline-support'
        }
      ]
    },
    {
      title: 'References',
      children: [
        {
          title: 'Options',
          link: '/options'
        },
        {
          title: 'Built-in Components',
          link: '/builtin-components'
        },
        {
          title: 'Plugin API',
          link: '/plugin-api'
        }
      ]
    },
    {
      title: 'Credits',
      link: '/credits'
    }
  ],
  overrides: {
    '/': {
      language: 'English'
    },
    '/zh/': {
      language: 'Chinese',
      editLinkText: '在 GitHub 上编辑此页',
      nav: [
        {
          title: '首页',
          link: '/zh/'
        },
        {
          title: 'GitHub',
          link: 'https://github.com/egoist/docute'
        }
      ],
      sidebar: [
        {
          title: '指南',
          children: [
            {
              title: '介绍',
              link: '/zh'
            },
            {
              title: '自定义',
              link: '/zh/guide/customization'
            },
            {
              title: 'Markdown 功能',
              link: '/zh/guide/markdown-features'
            },
            {
              title: '在 Markdown 中使用 Vue',
              link: '/zh/guide/use-vue-in-markdown'
            },
            {
              title: '国际化',
              link: '/zh/guide/internationalization'
            },
            {
              title: '插件',
              link: '/zh/guide/plugin'
            },
            {
              title: '部署',
              link: '/zh/guide/deployment'
            }
          ]
        },
        {
          title: '进阶',
          children: [
            {
              title: '使用打包工具',
              link: '/zh/guide/use-with-bundlers'
            },
            {
              title: '离线支持',
              link: '/zh/guide/offline-support'
            }
          ]
        },
        {
          title: '参考',
          children: [
            {
              title: '配置项',
              link: '/zh/options'
            },
            {
              title: '内置组件',
              link: '/zh/builtin-components'
            },
            {
              title: '插件 API',
              link: '/zh/plugin-api'
            }
          ]
        },
        {
          title: '致谢',
          link: '/zh/credits'
        }
      ]
    }
  },
  footer: `
  <div style="border-top:1px solid var(--border-color);padding-top:30px;margin: 40px 0;color:#999999;font-size: .9rem;">
  &copy; ${new Date().getFullYear()} Developed by <a href="https://egoist.sh" target="_blank">EGOIST</a>. Released under MIT license.
  </div>
  `,
  banner: {
    template: html`
      <div class="docute-banner">
        <note :label="false"
          ><SponsorIcon width="16" height="16" style="position:relative;top:2px;margin-right:8px;" />Sponsor the author on
          <a href="https://github.com/sponsors/egoist" target="_blank"
            >GitHub<ExternalLinkIcon /></a
          > to support Docute.</note
        >
      </div>
    `,
    components: {
      SponsorIcon
    }
  }
})

Vue.component('ReverseText', {
  props: {
    text: {
      type: String,
      required: true
    }
  },
  template: html`
    <div class="reverse-text">
      {{ reversedText }}
      <v-style>
      .reverse-text {
        border: 1px solid var(--border-color);
        padding: 20px;
        font-weight: bold;
        border-radius: 4px;
      }
      </v-style>
    </div>
  `,
  computed: {
    reversedText() {
      return this.text
        .split('')
        .reverse()
        .join('')
    }
  }
})

if (process.env.NODE_ENV === 'production' && 'serviceWorker' in navigator) {
  navigator.serviceWorker.register('/sw.js')
}
