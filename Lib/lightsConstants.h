#ifndef ALIENWARE_LIGHTS_CONSTANTS_H
#define ALIENWARE_LIGHTS_CONSTANTS_H


namespace alienware {




	//This is VIDs for different devices: Alienware (common), Darfon (RGB keyboards), Microship (monitors), Primax (mouses), Chicony (external keyboards)
	//enum Afx_VIDs {
	//	Aleinware = 0x187c,
	//	Darfon = 0x0d62,
	//	Microchip = 0x0424,
	//	Primax = 0x461,
	//	Chicony = 0x4f2
	//};
	//                           v0  1  2  3  4  5     6  7  8
	// const byte reportIDList[]{ 0, 2, 2, 2, 0, 0xcc, 0, 0, 0xe };










	// V4, common tron/desktop
	const byte COMMV4_control[]{6, 0x03 ,0x21 ,0x00 ,0x03 ,0x00 ,0xff };








	// [4] - control type (1..7), 1 - start new, 2 - finish and save, 3 - finish and play, 4 - remove, 5 - play, 6 - set default, 7 - set startup
	// [5-6] - control ID 0xffff - common, 8 - startup, 61 - light
	const byte COMMV4_colorSel[]{5, 0x03 ,0x23 ,0x01 ,0x00 ,0x01};
	// [3] - 1 - loop, 0 - once
	// [5] - count of lights need to be set,
	// [6-33] - LightID (index, not mask) - it can be COUNT of them.




	// [3] - action type ( 0 - light, 1 - pulse, 2 - morph)
	// [4] - how long phase keeps
	// [5] - mode (action type) - 0xd0 - light, 0xdc - pulse, 0xcf - morph, 0xe8 - power morph, 0x82 - spectrum, 0xac - rainbow
	// [7] - tempo (0xfa - steady)
	// [8-10]    - rgb
	// Then circle [11 - 17, 18 - 24, 25 - 31]
	// It can be up to 3 colorSet for one colorSel.
	const byte COMMV4_colorSet[]{7, 0x03 ,0x24 ,0x00 ,0x07 ,0xd0 ,0x00 ,0xfa};












	// Better use control with [2] = 22
	// [6] - type
	const byte COMMV4_setPower[]{2, 0x03 ,0x22};







	// [4] - brightness (0..100),
	// [5] - lights count
	// [6-33] - light IDs (like colorSel)
	const byte COMMV4_turnOn[]{2, 0x03, 0x26};






	// [3-5] - rgb
	// [7] - count
	// [8-33] - IDs
	const byte COMMV4_setOneColor[]{2, 0x03, 0x27 };






	// Operation codes for light mode
	const byte v4OpCodes[]{ 0xd0, 0xdc, 0xcf, 0xdc, 0x82, 0xac, 0xe8 };
}


#endif