
#include <iostream>
#include "lights.h"


using namespace std;
using namespace alienware;

int main() {
	Lights lights;

	cout << "[ .... ] Initializing" << endl;
	int pid = lights.Init(ALEINWARE_VID);
	if (pid < 0) {
		cout << "[ FAIL ] Initializing" << endl;
		return 0;
	}
	cout << "[  OK  ] Initializing" << endl;

	ArrayDeque<LightBlock *> list;

//	Action color1 = { AlienFX_A_Color, 160, 160,   0, 100, 255 };
//	Action color2 = { AlienFX_A_Color, 160, 160, 255,  70,   0 };

	Action color1 = { AlienFX_A_Color, 160, 160,   0, 100, 255 };
	Action color2 = { AlienFX_A_Color, 160, 160, 255,  70,   0 };
	Action color3 = { AlienFX_A_Color, 160, 160,   0,   0,   2 };


	LightBlock kb0 = { 0 };
	kb0.act.Add(&color1);

	LightBlock kb1 = { 1 };
	kb1.act.Add(&color1);

	LightBlock kb2 = { 2 };
	kb2.act.Add(&color1);

	LightBlock kb3 = { 3 };
	kb3.act.Add(&color2);

	LightBlock bar0 = { 0x4 };
	bar0.act.Add(&color3);

	LightBlock bar1 = { 0x5 };
	bar1.act.Add(&color3);

	LightBlock bar2 = { 0x6 };
	bar2.act.Add(&color3);

	LightBlock bar3 = { 0x7 };
	bar3.act.Add(&color3);

	LightBlock bar4 = { 0x8 };
	bar4.act.Add(&color3);

	LightBlock bar5 = { 0x9 };
	bar5.act.Add(&color3);

	LightBlock bar6 = { 0xA };
	bar6.act.Add(&color3);

	LightBlock bar7 = { 0xB };
	bar7.act.Add(&color3);

	LightBlock bar8 = { 0xC };
	bar8.act.Add(&color3);

	LightBlock bar9 = { 0xD };
	bar9.act.Add(&color3);

	LightBlock barA = { 0xE };
	barA.act.Add(&color3);

	LightBlock barB = { 0xF };
	barB.act.Add(&color3);


	list.Add(&kb0);
	list.Add(&kb1);
	list.Add(&kb2);
	list.Add(&kb3);
	list.Add(&bar0);
	list.Add(&bar1);
	list.Add(&bar2);
	list.Add(&bar3);
	list.Add(&bar4);
	list.Add(&bar5);
	list.Add(&bar6);
	list.Add(&bar7);
	list.Add(&bar8);
	list.Add(&bar9);
	list.Add(&barA);
	list.Add(&barB);

	cout << "[ .... ] Updating lights" << endl;
	if (!lights.Update(&list, true)) {
		cout << "[ FAIL ] Updating lights" << endl;
		return 0;
	}
	cout << "[  OK  ] Updating lights" << endl;

	return 0;
}