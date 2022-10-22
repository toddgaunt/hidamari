# Hidamari
Hidamari is a small implementation of a certain falling-block game that aims to
be as official-guideline compliant as possible. You can select to play the game
yourself, or navigate in the (incomplete) menu to enable the AI.

## Building and running
Please make sure you have a C compiler, SDL2 library & headers, and SDL2_image
library & headers installed before compiling the project:

On Fedora you can run `sudo dnf install SDL2_image SDL2_image-devel SDL2 SDL2-devel`

Finally just run `make clean all` to build the binary, and then execute it to
play.

#### Controls
| Action                   | Key                               |
|--------------------------|-----------------------------------|
| move down                | s, k, down-arrow                  |
| move left                | a, j, left-arrow                  |
| move right               | d, l, right-arrow                 |
| rotate clockwise         | e, o, x, up-arrow                 |
| rotate counter-clockwise | q, u, left-control, right-control |
| hard drop                | space, return                     |

#### Menu
The main menu doesn't have selection highlighting at the moment, but can
be navigated with the arrow keys and space/return.
