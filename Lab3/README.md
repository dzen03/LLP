# Lab3

## Installation
You need to get protobuf-c + protobuf: either binary or build from https://github.com/protobuf-c/protobuf-c
Also you need to edit [CMakeLists.txt](CMakeLists.txt): PROTOBUF_ROOT, ABSL_ROOT, PROTOBUF_C_ROOT

## Tests
From [link](https://demo.neo4jlabs.com:7473/browser/?dbms=neo4j://northwind@demo.neo4jlabs.com&db=northwind). Login&pass are northwind

Will execute:
```
MATCH (c: Customer)-[PURCHASED]->(o:Order) WHERE toInteger(o.shipPostalCode) is not null RETURN distinct c.contactName as name, toInteger(o.shipPostalCode) as zip, o.shipCountry as country
```

And it will get us [records.json](test/records.json)

Then we will test:
```
MATCH (c: Customer)-[PURCHASED]->(o:Order) WHERE toInteger(o.shipPostalCode) is not null and toInteger(o.shipPostalCode) < 90000 and o.shipCountry = "USA" RETURN distinct c.contactName as name, toInteger(o.shipPostalCode) as zip, o.shipCountry as country

vs

select {
    customer {
        name
        purchased (country: "USA", zip < 90000) {
            country
        }
    }
}

``` 
Canonical data:

|   | name                 | zip   | country |
|---|----------------------|-------|---------|
| 1 | "Paula Wilson"       | 87110 | "USA"   |
| 2 | "Art Braunschweiger" | 82520 | "USA"   |
| 3 | "Jose Pavarotti"     | 83720 | "USA"   |
| 4 | "Liu Wong            | 59801 | "USA"   |


I've got:
``` 
{(name: "Paula Wilson")} -- [purchased] -- {(country: "USA")(zip: 87110)}
{(name: "Jose Pavarotti")} -- [purchased] -- {(country: "USA")(zip: 83720)}
{(name: "Art Braunschweiger")} -- [purchased] -- {(country: "USA")(zip: 82520)}
{(name: "Liu Wong")} -- [purchased] -- {(country: "USA")(zip: 59801)}
```

*IT WORKS!!!*
