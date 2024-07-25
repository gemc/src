# Python API to build detectors 

This interface is part  [GEMC](https://gemc.github.io/home/).
It provides utilities to build detectors, create materials, 
assign sensitivity, etc.

### Prerequisites:

- python3
- gemc installation

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
