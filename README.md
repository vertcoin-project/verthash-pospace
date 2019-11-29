# Verthash PoSpace Proof-of-concept

Add some description here

## Building

```
git clone https://github.com/metalicjames/verthash-pospace
cd verthash-pospace
git pull --recurse-submodules --jobs=10
make all
```

## Running

First, run h1 to generate the file. On a Core i7 with SSD this should take ~5 minutes.

```
cd <folder where verthash-pospace is checked out>
./h1
```

Then, you can run the trial hashing of a dummy header using h2.

```
cd <folder where verthash-pospace is checked out>
./h2
```