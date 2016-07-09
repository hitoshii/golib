
all: log

log: src/log/log.go
	GOPATH=`pwd` go build src/log/log.go

fmt:
	find . -name '*.go'|xargs gofmt -w

expl: example/log.go
	GOPATH=`pwd` go build -o bin/log example/log.go

