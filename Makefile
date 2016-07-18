
GOPATH=`pwd`

all: log unix

fmt:
	find . -name '*.go'|xargs gofmt -w

log: src/log/log.go
	go build $^

unix: src/unix/unistd.go
	go build $^

example: expl-log expl-unix

expl-log: example/log.go
	go build -o bin/log $^

expl-unix: example/unix.go
	go build -o bin/unix $^

