package alienware

/*
	#include <stdlib.h>
	#include <stdbool.h>
	#include "../../AlienwareCpp/AlienwareLights.h"

	#cgo LDFLAGS: -L../../AlienwareCpp/cmake-build-release -lAlienware64

*/
import "C"
import (
	"fmt"
	"unsafe"
)

type LightActionType byte

const (
	LightActionColor     LightActionType = 0
	LightActionPulse     LightActionType = 1
	LightActionMorph     LightActionType = 2
	LightActionBreathing LightActionType = 3
	LightActionSpectrum  LightActionType = 4
	LightActionRainbow   LightActionType = 5
	LightActionPower     LightActionType = 6

	LightActionStructSize = 6
)

var lightsCount = 0

type LightAction struct { // atomic light action phase
	Type  LightActionType // one of Action values - action type
	Time  byte            // How long this phase stay
	Tempo byte            // How fast it should transform
	R     byte
	G     byte
	B     byte
}

type LightConfig []*LightAction

func InitLaptop(count uint32) bool {
	lightsCount = int(count)
	return bool(C.AlienwareInitLaptop(C.uint(count)))
}

func Free() {
	C.AlienwareFree()
}

func Update(config *[]LightConfig, save bool) bool {
	if len(*config) != lightsCount {
		panic("len(config) must be equal to the number of lights")
	}
	bufSize := lightsCount
	for i := 0; i < lightsCount; i++ {
		bufSize += LightActionStructSize * len((*config)[i])
	}
	buf := C.malloc(C.ulonglong(bufSize))
	mem := buf
	for i := 0; i < lightsCount; i++ {
		l := len((*config)[i]) // light length
		buf = unsafe.Add(buf, 1)
		*((*byte)(buf)) = byte(l)
		for j := 0; j < l; j++ {
			pAction := (*config)[i][j]
			fmt.Printf("%d typ: %d tim: %d tmp: %d R: %d G: %d B: %d\n", i, pAction.Type, pAction.Time, pAction.Tempo, pAction.R, pAction.G, pAction.B)
			*(*byte)(buf) = byte(pAction.Type)
			buf = unsafe.Add(buf, 1)
			*(*byte)(buf) = pAction.Time
			buf = unsafe.Add(buf, 1)
			*(*byte)(buf) = pAction.Tempo
			buf = unsafe.Add(buf, 1)
			*(*byte)(buf) = pAction.R
			buf = unsafe.Add(buf, 1)
			*(*byte)(buf) = pAction.G
			buf = unsafe.Add(buf, 1)
			*(*byte)(buf) = pAction.B
			buf = unsafe.Add(buf, 1)
		}
	}
	result := bool(C.AlienwareUpdateLights((*C.char)(mem), C.bool(save)))
	C.free(mem)
	return result
}
