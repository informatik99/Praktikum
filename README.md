# BVS Server
Dieser Server wurde für das Praktikum 
im Fach "Betriebssysteme und Verteilte Systeme" 
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
```
## Coding Style
```c

#define PREPROCESSOR_MACROS_CONSTANTS_UPPERCASE

typedef enum {
    ENUMS_EVERYTHING_UPPERCASE
} EnumTypesUpperCamelCase;

typedef struct StructUpperCamelCase {
    int memberVariableLowerCamelCase;
    char anotherLowerCamelCaseVariable;
} StructUpperCamelCase; 


int prefix_obj_action(AnyType *obj);
int obj_action(SomeOtherType *obj)


```
