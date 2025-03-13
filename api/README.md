
### Prerequisites:

- python3

# Python API to build detectors 

This [GEMC](https://gemc.github.io/home/) API provides utilities to build detectors, create materials, 
assign sensitivity, etc. All the information is stored in databases.

The supported databases are:

- ascii text files
- SQLite database

``
# Quickstart

The python script `templates.py` can be used to create: 
 - a new system 
 - template code to build a new volume, material, etc.


### New system 

To create new system:

``` 
$GEMC/api/templates.py -s my_system
```

See [gemc systems](https://gemc.github.io/home/documentation/system) for more details.
