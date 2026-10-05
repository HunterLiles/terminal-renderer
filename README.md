# Terminal Renderer

Simple 3D renderer inside the terminal, written in C with no external dependencies. Supports both rasterization and raymarching with signed distance fields, all running on the CPU. The image is made with Braille characters, with ANSI codes for colors and the TUI.

## Raymarcher

<table>
  <tr>
    <td align="center" width="50%">
      <img src="screenshots/raymarcher/torus.gif" alt="Raymarched torus SDF" width="440">
      <br>
      <sub>Torus SDF</sub>
    </td>
    <td align="center" width="50%">
      <img src="screenshots/raymarcher/displaced-sphere.gif" alt="Raymarched sphere SDF with surface displacement" width="440">
      <br>
      <sub>Sphere SDF with surface displacement</sub>
    </td>
  </tr>
</table>

## Rasterizer

<table>
  <tr>
    <td align="center" width="50%">
      <img src="screenshots/rasterizer/torus.gif" alt="Rasterized torus" width="440">
      <br>
      <sub>Torus</sub>
    </td>
    <td align="center" width="50%">
      <img src="screenshots/rasterizer/cube.gif" alt="Rasterized cube" width="440">
      <br>
      <sub>Cube</sub>
    </td>
  </tr>
</table>
