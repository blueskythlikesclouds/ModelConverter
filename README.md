# Model Converter

Model Converter is a tool that converts 3D model formats like FBX to Hedgehog Engine .model files.

Are you looking into converting .model files to FBX instead? [See modelfbx in libgens-sonicglvl.](https://github.com/DarioSamo/libgens-sonicglvl.git)

## Usage

```
ModelConverter [options] [source] [destination]
```

If the destination is not specified, the model file will be saved in the same folder as the source file.

### Options

* `--unleashed`
	* Convert to Sonic Unleashed format.
* `--gens`
	* Convert to Sonic Generations format.
* `--lw`
	* Convert to Sonic Lost World format.
* `--forces`
	* Convert to Sonic Forces format.
* `--frontiers`
	* Convert to Sonic Frontiers format.
* `--override-materials` or `-y`
	* Replace any existing materials in the output directory with the new ones.
* `--no-pause` or `-np`
	* Do not pause the console if an error occurs.
	
### Examples

```
ModelConverter --frontiers chr_sonic.fbx chr_sonic.model
```

You can also drag and drop your 3D model file onto the included .bat files to convert without having to use the command line.

## Tags

You can customize the way the model and materials are exported by adding optional tags to the end of mesh or material names.

### Mesh Tags

* `NAME`: Sets the name of the mesh. This is usually used to manage mesh visibility, such as Sonic’s mouth.
	* Example: `@NAME(Sonic_Mouth)`
* `PRP`: Adds Lost World/Forces/Frontiers specific properties to the model. Can be used multiple times.
	* Example: `@PRP(ShadowRe, true)@PRP(ShadowCa, true)`

#### Example

```
mouthL@NAME(Sonic_Mouth_L)@PRP(ShadowRe, true)@PRP(ShadowCa, true)
```

### Material Tags

These function the same way as [Hedgehog Converter](https://github.com/DarioSamo/libgens-sonicglvl).

* `SHDR`: Sets the shader for the material.
	* Example: `@SHDR(Common_d)`
* `LYR`
	* Sets the rendering layer for the material, which affects how the mesh is displayed. The possible values are:
		* `solid`: The surface is opaque and ignores the alpha channel in the texture. This is the default value.
		* `trans`: The surface is transparent/see-through depending on the alpha channel in the texture. This should be used sparingly as it can cause depth sorting issues.
		* `punch`: The surface has punch-through transparency and is either completely opaque or invisible depending on the alpha channel in the texture. This should be used with textures that have sharp edges (for example leaves).
	* Example: `@LYR(punch)`
* `CULL`:
	* Enables or disables backface culling for the surface. The possible values are:
		* `true`: The surface is only visible from the front. This is the default value.
		* `false`: The surface is visible from both sides.
	* Example: `@CULL(false)`
* `ADD`: Makes the surface additive and gives it a glowing effect.
	* Example: `@ADD(true)`
* `TXTR`: Adds a texture to the material with or without the file extension. Can be used multiple times.
	* Example: `@TXTR(specular, chr_sonic_body01_spc_HD)`
	* Note: Material textures in the 3D model file also are added to the resulting material, so be careful not to add duplicates with this tag.
* `PMTR`: Adds a parameter of 4 float values to the material. Can be used multiple times.
	* Example: `@PMTR(g_SonicSkinFalloffParam, 0.15, 2, 3, 0)`
* `PRP`: Adds Lost World/Forces/Frontiers specific properties to the material. Can be used multiple times.
	* Example: `@PRP(TrnsPrio, 5)`
	
### Example

```
sonic_gm_cloth@SHDR(ChrSkin_dsn)@TXTR(specular, chr_sonic_cloth_spc_HD)@TXTR(normal, chr_sonic_cloth_nrm_HD)@PMTR(g_SonicSkinFalloffParam, 0.1, 0.5, 3, 0)
```

## Remarks

* When exporting from Blender, use the following settings:
	* Transform -> Apply Scalings: FBX Units Scale
	* Armature -> Add Leaf Bones: Unchecked
*  The model and the skeleton file must have the same number of nodes, otherwise the animations will not work properly. Make sure your input 3D model file only contains the skeleton and the meshes, because the tool will also convert any extra nodes that are not rigged.