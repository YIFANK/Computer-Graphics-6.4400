I compiled and ran my code on MacOS. I did not collaborate with anyone else. I found this assignment to be really challenging, and I spent a lot of time debugging my lighting direction because it was not working. After asking on Piazza, I discovered that I was using the normal instead of the perfect reflection ray for speculative light. Then I changed that and got most things fixed.

I implemented antialiasing using supersampling, and I also implemented a "fisheye" camera that has some fancy effects:

here are the scripts:
```
./assignment4 -input scene01_plane.txt -output 01.png -size 800 800
./assignment4 -input scene02_cube.txt -output 02.png -size 800 800
./assignment4 -input scene03_sphere.txt -output 03.png -size 800 800
./assignment4 -input scene04_axes.txt -output 04.png -size 800 800
./assignment4 -input scene05_bunny_200.txt -output 05.png -size 800 800
./assignment4 -input scene06_bunny_1k.txt -output 06.png -size 800 800 -bounces 4
./assignment4 -input scene07_arch.txt -output 07.png -size 800 800 -shadows -bounces 4 
./assignment4 -input scene07_arch.txt -output anti07.png -size 800 800 -shadows -bounces 4 -samples 16
./assignment4 -input scene06_bunny_1k.txt -output fisheye.png -size 800 800 -bounces 4 -camera_type fisheye -samples 8
```

add additional argument `-camera_type fisheye` to use fisheye camera.