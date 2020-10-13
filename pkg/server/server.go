package server

import (
  "net/http"
)

func Start() {
  http.Handle("/", http.FileServer(http.Dir("./web")))
  http.ListenAndServe(":8000", nil)
}
