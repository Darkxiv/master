#ifndef __SETTINGS_H
#define __SETTINGS_H

#define WIN_W 1440
#define WIN_H 900

#define WIN_POS_X 0
#define WIN_POS_Y 0

#define TIMER_SPEED 1
#define GSS_ERROR 1

#define SPHERE_SHAPE 50
#define LIGHT_SPHERE_SHAPE 15

#define MOTION_CALL -1

#define M_PI 3.14159265359f
#define EPS 0.00001

#define COPYRIGHT "This demo was created by Dontsov Valentin for MailRu Group and Allods team."
#define TEXTURE_PATH "data/textures/"
#define GREETING COPYRIGHT "\nCommands:\n" \
	"\tq / [ESC]- Quit the application\n" \
	"\twasd\t- Move ball forward, back, left, right relative to the camera\n" \
	"\tz/x\t- Decrease/Increase ball's shininess\n" \
	"\tc/v\t- Decrease/Increase ball's reflectivity\n" \
	"\tb\t- Enable/Disable motion blur\n" \
	"\tl\t- Show/hide light sources\n" \
	"TIP: Use english keyboard layout\n"
#endif