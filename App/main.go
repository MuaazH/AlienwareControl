package main

import (
	"Alienware/alienware"
	"encoding/json"
	"os"
)

type Config struct {
	Lights *[]alienware.LightConfig
}

func main() {
	data, err := os.ReadFile("config.json")
	if err != nil {
		panic(err)
	}
	var conf Config
	err = json.Unmarshal(data, &conf)
	if err != nil {
		panic(err)
	}

	if !alienware.InitLaptop(uint32(len(*conf.Lights))) {
		println("could not init lights")
		return
	}
	if !alienware.Update(conf.Lights, true) {
		println("could not update lights")
	}
	alienware.Free()
}
