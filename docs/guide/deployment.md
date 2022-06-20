# Deployment

Keep in mind, it's just a static HTML file that can be served anywhere.

## Vercel <Badge type="success">Recommended</Badge>

[Vercel](https://zeit.co) is a platform for Global Static, Jamstack and Serverless Deployments.

On the Vercel Dashboard, you can import a project from Git Repository, and the only field you need to fill is `output directory`, which should be the directory to your `index.html`.

<ImageZoom src="https://user-images.githubusercontent.com/8784712/81292070-88f14580-909d-11ea-9c69-ed9d1854cad2.png" alt="vercel deploy" border />

## Render

Render offers [free static site hosting](https://render.com/docs/static-sites) with fully managed SSL, a global CDN and continuous auto deploys from GitHub.

1. Create a new Web Service on Render, and give Render’s GitHub app permission to access your new repo.
2. Use the following values during creation:
  - Environment: Static Site
  - Build Command: Leave it blank
  - Publish Directory: The directory to your `index.html`, for example it should be `docs` if you populated it at `docs/index.html`.

## Netlify

1. Login to your [Netlify](https://www.netlify.com/) account.
2. In the [dashboard](https://app.netlify.com/) page, click __New site from Git__.
3. Choose a repository where you store your docs, leave the __Build Command__ area blank, fill in the __Publish directory__ area with the directory of your `index.html`, for example it should be `docs` if you populated it at `docs/index.html`.

## GitHub Pages

The easiest way to use GitHub Pages is to populate all your files inside `./docs` folder on the master branch, and then use this folder for GitHub Pages:

<ImageZoom src="https://i.loli.net/2018/06/11/5b1e0da0c173a.png" alt="github pages" :border="true" />

However you can still use `gh-pages` branch or even `master` branch to serve your docs, it all depends on your needs.

<Note type="warning">

You need to populate a `.nojekyll` file (with empty content) in that folder to disable GitHub Pages' default behaviors for the Jekyll framework (which we don't use at all).

</Note>
