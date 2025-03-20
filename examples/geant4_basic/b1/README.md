

|  [![gemc][gemc-logo]]([gemc-home])  |
|:-----------------------------------:|
|       Geant4 Basic Example B1       |


### Description

This example reproduces the [Geant4 example B1]([example-B1]). 
The world is a box containing an envelope made of water, also a box.  

The envelope includes two volumes: a spherical cone and a trapezoid. The two inner volumes
are made from tissue and bone materials: 

- G4_A-150_TISSUE
- G4_BONE_COMPACT_ICRU

The world volume is defined in the yaml file:

```yaml
root: G4Box, 12*cm, 12*cm, 18*cm, G4_WATER
```

### Physics List

QBBC is used, selected in the yaml file:

```yaml
phys_list: QBBC
```

### Generator

### Usage

- #### Building the detector

- #### Running gemc

- #### Examples

### Output

### Notes




## Author(s)
| M. Ungaro | [![Homepage][home-icon]]([homepage])  | [![email][email-icon]]([email]) |  [![github][gh-icon]]([github])   |
|:---------:|:-------------------------------------:|:-------------------------------:|:---------------------------------:|


[example-B1]: https://geant4-userdoc.web.cern.ch/Doxygen/examples_doc/html/ExampleB1.html
[home-icon]: https://cdn3.iconfinder.com/data/icons/feather-5/24/home-32.png
[email-icon]: https://cdn4.iconfinder.com/data/icons/aiga-symbol-signs/439/aiga_mail-32.png
[gh-icon]: https://cdn4.iconfinder.com/data/icons/ionicons/512/icon-social-github-32.png

[homepage]: https://maureeungaro.github.io/home/
[email]: mailto:ungaro@jlab.org
[github]: https://github.com/maureeungaro

[gemc-logo]: https://github.com/gemc/home/blob/main/assets/images/gemcLogo64.png?raw=true
[gemc-home]: https://gemc.github.io/home/