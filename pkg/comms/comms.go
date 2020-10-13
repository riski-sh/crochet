package comms

import (
  "log"
  "encoding/json"
  "encoding/binary"
  "os"
  "io/ioutil"
  "fmt"
)

var pipeWrite *os.File
var pipeRead *os.File

func sendPacket(pkt *RawPacket) {
  log.Printf("=> %s", pkt.Data)

  err := binary.Write(pipeWrite, binary.LittleEndian, pkt.Size)
  if err != nil {
    log.Fatal(err)
  }

  _, err = pipeWrite.Write(pkt.Data)
  if err != nil {
    log.Fatal(err)
  }
}

func Connect() {
  log.Println("reading pid data from crochet.pid");

  file, err := ioutil.ReadFile("crochet.pid")
  if err != nil {
    log.Fatal(err)
  }
	data := PIDData{}

	_ = json.Unmarshal([]byte(file), &data)

  log.Printf("will read from /proc/%d/fd/%d", data.PID, data.Read);
  log.Printf("will write to /proc/%d/fd/%d", data.PID, data.Write);

  var readFileString string = fmt.Sprintf("/proc/%d/fd/%d", data.PID, data.Read)
  var writeFileString string = fmt.Sprintf("/proc/%d/fd/%d", data.PID, data.Write)

  var openPipeErr error
  pipeRead, openPipeErr = os.Open(readFileString)

  if openPipeErr != nil {
    log.Fatal(openPipeErr)
  }

  pipeWrite, openPipeErr = os.OpenFile(writeFileString, os.O_WRONLY, os.ModeAppend)
  if openPipeErr != nil {
    log.Fatal(openPipeErr)
  }

  log.Println("ipc comms with crochet backend success")

  var raw RawPacket
  raw.Data = ([]byte)("hello")
  raw.Size = int32(len(raw.Data))

  sendPacket(&raw)

}
