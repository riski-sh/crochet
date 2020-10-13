package main

import (
  "github.com/riski-sh/crochet/pkg/comms"
  "github.com/riski-sh/crochet/pkg/server"
)

func main() {
  comms.Connect()
  server.Start()
}
