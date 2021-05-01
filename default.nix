with import <nixpkgs> { };

stdenv.mkDerivation {
  name = "evmerge-0.0.1";
  src = ./.;

  buildInputs = [ libevdev ];

  installPhase = ''
    mkdir -p $out/bin
    cp evmerge $out/bin
    cp dobuttons $out/bin
  '';

  meta =  {
    description = "Linkx web browser";
    license = "GPLv2";
  };
}
