# Current Activities & Next Steps

## TODO Rules
1. Completed items are striked through and moved to the "Recently Completed" section if possible. You can strike throught subitems too. e.g: ~~Completed item or subitem~~
2. Write priority levels for in-progress items (High, Medium, Low) and sort them by priority.
3. For in-progress items, write clear next steps or sub-tasks to complete them.

## In Progress / Next Steps

### Rendering System (Priority: High)
1. **Dynamic Lighting System**
   - Implement point light support in shader system
   - Implement spot light support with cone calculations
   - Light culling based on distance and render layers
   - Per-object light assignment

2. **Advanced Shadow Mapping** 
   - Shadow map texture generation (depth pass)
   - Cascaded shadow maps for better quality
   - Percentage-closer filtering (PCF) for soft shadows
   - Shadow bias and slope-scale bias adjustment
   - Support for multiple shadow-casting lights

3. **Material System**
   - Basic material properties (albedo, roughness, metallic)
   - Material-based lighting calculations
   - Custom shader support for materials
   - Dynamic vs static object batching

4. **Optimization**
   - Object culling (frustum, occlusion)
   - Batch rendering for same material/geometry
   - GPU instancing for repeated objects

5. **Make `receiveShadows` work**
    - Ensure shadow receiver objects properly display shadows cast by other objects

6. **Fix shadows are anywhere problem**
    - Ensure shadows are only cast on the objects which allows receiving shadows, not emptiness or other objects that don't receive shadows

### Graphics Quality (Priority: Medium)
1. **Anti-aliasing**
   - MSAA support via backend
   - Post-process AA options

2. **Post-Processing**
   - Bloom/HDR
   - Tone mapping
   - Color grading

### Asset System (Priority: Medium)
1. Custom mesh loading (replace PrimitiveType::Custom)
2. Texture loading and management
3. Material asset format and loading

-- Progress: Added basic `ResourceManager` for raw file loading (non-blocking).

### Testing & Validation
1. Create comprehensive rendering test scenes
2. Performance profiling and optimization
3. Cross-API testing (D3D11, Vulkan, OpenGL)

### Documentation (Priority: Low)
1. Lighting system guide
2. Shadow system tutorial
3. Material creation guide