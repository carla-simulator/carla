# Category `pages` — reviewing and rewriting a tutorial page

For a page under `Docs/` that a person reviews by hand (a tutorial, a content-authoring page, a tool page), as opposed
to the generated reference. These rules come from the page-by-page review of the UE 5.8 docs, where the reviewers
corrected the same mistakes more than once.

## Workflow per page

1. **Read the page, then verify every claim** against the code or the content, with a `file:line` or an asset path
   for each one. A claim you cannot verify is either removed or written as unverified in the report, never kept as
   fact. Material, blueprint and level facts come from the assets themselves (editor Python, see
   [images.md](images.md#editor-recipes)), not from names or old images.
2. **Report before editing**: errors, what is missing, format problems, and whether the page needs an agent check.
   Apply only after the reviewer agrees.
3. **Agent check**: execute the page as written on a real server or editor, with nothing saved. `git status` of the
   code repository and of `Unreal/CarlaUnreal/Content/Carla` must be identical before and after, and no temporary
   asset folder may be left on disk. Report it as *OK* only when the page itself ran; *Launched* when only part of
   it could run.
4. **A page that describes something that no longer exists is rewritten, not deleted**: describe the current state,
   with diagrams and real captures, and leave it marked for an expert of that area.
5. Commit only when the reviewer says so.

## Writing

- **Document what is current.** When the content holds an old and a new generation of the same thing (for
  materials, `GenericMaterials/00_MastersOpt` and `00_LegacyMaterials` are the UE4-era masters, `000_Masters` the
  current ones), document only the current one, even if old assets still use the old one. Ask the reviewer when
  usage counts and the folder's age disagree.
- **Only the columns the reader acts on.** A table column that repeats a folder path for every row (`Path in ...`)
  is noise; say the folder once in the text.
- **When another page already explains a subject in depth**, write two or three lines and link to it.
- **Name the channel.** Write "the `R` channel of the ORME texture", "the BaseColor alpha channel", in the text and
  in the image titles, never a bare "ORME R" or "BaseColor alpha".
- **Do not document typos in asset or parameter names.** Use the name exactly as it is (`Emissive atenuance`,
  `Difuse texture`) and say nothing about the spelling: fixing the content is a separate task.
- **Say what a parameter is for, from the user's side.** Example: the `EmissiveIntensity` and `On/Off` parameters
  of the *API Values* groups simulate the input that the CARLA API sends at runtime, so an artist can preview the
  lit material in the editor without launching PIE; `Emissive attenuance` divides the general emissive value, which
  comes from the API through `BP_Lights` and is global to every emissive material of that light, to tune one
  instance.
- **Examples must not mislead.** If an example instance was made for the picture (textures of an existing asset in
  another master, a demo placed in a town), the caption says so.

## Material pages

The structure that the reviewers asked for, per master material:

1. What the master is for (one paragraph: general master for most assets, simple master as an ultra-light master
   for assets that need no color variation, emissive or weather, glass for transparent glass, and so on).
2. A panel with the textures it reads, each channel separated and titled with its channel.
3. A crop of each parameter group of the material instance editor, with its explanation right below it
   (`2. UVs`: what `World Position` and `Triplanar` do; `4. Color Corrections`: `Brightness`, ...). Tick the
   switches that hide other parameters in an unsaved copy of the instance first, so every parameter is visible.
4. Examples where they help, in a CARLA town with the CARLA camera: variants of the same asset (color, saturation,
   brightness), emissive on and off by day and night, decal shown and hidden, dry and wet road.

Choosing the example asset:

- **Check the masks before picking textures.** Most UE5-era building textures have a flat alpha (all white or all
  black), which shows nothing. Export the candidates and measure the alpha (`std` of the channel) first; the
  UE4-era houses (`House08`) have real color and emissive masks.
- **Emissive examples on assets that really emit**: Town10 buildings whose instance has `Emissive Texture` ticked
  (the `Shop02` sign of Skysc19, `MI_Apt20_withEmissive`) or fake interiors, which CARLA lights at night. An
  emissive mask that covers a few small lamps shows no difference between options.
- **Render the effect before describing it.** `Brightness`, `Contrast` and similar corrections were described wrong
  from their names until a render showed what they do.
- **Weather in examples uses the presets** (`carla.WeatherParameters.WetNoon` and so on), not extreme values:
  `precipitation_deposits` 90 floods the whole road and hides it. Adjust one or two values of the preset if needed (the
  road page uses `WetNoon` with `precipitation_deposits` 40 and `wetness` 80: without `wetness` the asphalt between
  puddles stays dry) and say so in the caption.
- **No real people or brands in examples.** Some advertising textures in the content show a real person; pick
  another one.

## Images

- Animations: animated WebP, never GIF. A slideshow of grids holds each frame 3 s.
- A screenshot of a tool panel goes cropped, right above the table or list that explains it.
- Board or tracker notes for other people are one sentence.
