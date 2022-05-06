# BVS Server

Dieser Server wurde für das Praktikum im Fach "Betriebssysteme und Verteilte Systeme"
erstellt

## Ausführung

### Starten

- server starten
- mit telnet auf port 5678 verbinden

```shell
telnet localhost 5678
```

### Befehle

```
PUT key value
GET key
DEL key
SUB key
UNSUB key
BEG
END
QUIT
```

## Coding Style

- Macros, Konstanten groß: `MAX_BUFFER_SIZE`
- Structs und typedefs CamelCase: `DoorFactory`
- funktionen die sich auf structs beziehen: `struct_action(...);`

