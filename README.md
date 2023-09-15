#  Ottergon
[![ubuntu 20.04](https://github.com/duckstax/ottergon/actions/workflows/ubuntu_20_04.yaml/badge.svg)](https://github.com/duckstax/ottergon/actions/workflows/ubuntu_20_04.yaml)
[![manylinux2014](https://github.com/duckstax/ottergon/actions/workflows/manylinux2014.yml/badge.svg)](https://github.com/duckstax/ottergon/actions/workflows/manylinux2014.yml)
#  Ottergon is a development platform for building analytical systems and applications.

### Enjoy easy programming with Ottergon!

### Installation

#### Python example:

```bash
    pip install ottergon==0.5.0 
```

### Python SQl example:

```python
    from ottergon import Client

    client = Client()
    database = client["MyDatabase"]
    collection = database["MyCollection"]
    collection.execute("INSERT INTO MyDatabase.MyCollection (object_name, count ) VALUES ('object value', 1000)")
    collection.execute("SELECT * FROM MyDatabase.MyCollection WHERE object_name = 'object value' ")
```


### Python NoSQl example:

```python
    from ottergon import Client

    client = Client()
    database = client["MyDatabase"]
    collection = database["MyCollection"]
    collection.insert_one({"object_name": "object value", "count": 1000})
    collection.find_one({"object_name": "object value"})
```

### C++ SQL example:

```cpp
    auto config = create_config("/tmp/my_collection");
    spaces_t space(config);
    auto* dispatcher = space.dispatcher();
    dispatcher->execute_sql("INSERT INTO MyDatabase.MyCollection (object_name, count ) VALUES ('object value', 1000)");
    auto value = dispatcher->execute_sql("SELECT * FROM MyDatabase.MyCollection WHERE object_name = 'object value' ");
```

### C++ NoSQL example:

```cpp
    auto config = create_config("/tmp/my_collection");
    spaces_t space(config);
    auto* dispatcher = space.dispatcher();
    dispatcher->insert_one("MyDatabase", "MyCollection", {"object_name": "object value", "count": 1000});
    auto value = dispatcher->find_one("MyDatabase", "MyCollection", {"object_name": "object value"});
```

## Major futures of the project

* In-process
* serverless
* Persistence index
* Persistence storage
* Write-ahead log

## Coming soon

* APIs for Rust, Go, R, Java, etc.
* Vectorized engine
* Optimized for analytics
* Parallel query processing
* Transactions
* SQL support
* Parquet / CSV / ORC
* Column storage

## Getting involved

Even if you do not plan to contribute to duckstax ottergon itself or ottergon
integrations in other projects, we'd be happy to have you involved:

- Follow our activity on [GitHub issues][1]
- Contribute code to one of the reference implementations

[1]: https://github.com/duckstax/ottergon/issues
[2]: https://github.com/duckstax/ottergon/

