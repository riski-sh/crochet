package comms

type RawPacket struct {
	Size int32
	Data []byte
}

type PIDData struct {
	PID   int `json:"pid"`
	Write int `json:"write"`
	Read  int `json:"read"`
}
