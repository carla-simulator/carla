# AI Rendering

CARLA's integrated AI rendering techniques expand the possibilities for creating photorealistic, highly diverse simulation datasets using neural networks trained on large real-world datasets.

CARLA currently supports 2 AI rendering techniques:

* [Neural reconstruction with NVIDIA Omniverse's NuRec tool](nvidia_nurec.md)
* [Style transfer using NVIDIA's Cosmos Transfer1 world foundation model](nvidia_cosmos_transfer.md) 

---

## Neural reconstruction

NVIDIA's neural reconstruction technology enables a rich 3D representation of an environment to be learned by neural networks from a sequence of sensor data captured in the real world (such as a sequence of 2D camera images or LIDAR data). Variations and randomizations can then be applied in the 3D representation, such as perturbing the trajectory or adjusting the sensor configuration before re-simulation. This facilitates the generation of a diverse range of perturbations of a scenario using only one sequence of recorded sensor data. Neural reconstruction is a powerful tool to diversify training data or testing scenarios. Please refer to the [Neural reconstruction documentation](nvidia_nurec.md) for details on how to install and use the tool.

## Cosmos Transfer

NVIDIA's Cosmos Transfer is a branch of Cosmos World Foundation Models (WFMs) specialized for multimodal controllable conditional world generation or world2world transfer. Cosmos Transfer is designed to bridge the perceptual divide between simulated and real-world environments.

Users can generate endless hyper-realistic video variations from CARLA sequences using simple text prompts. This capability is ideal for:

* Expanding visual diversity in perception datasets
* Bridging the domain gap for sim-to-real training
* Exploring edge cases with photorealistic textures, lighting, and weather variations

Please refer to the [Cosmos Transfer documentation](nvidia_cosmos_transfer.md) for details on how to use this tool with CARLA.