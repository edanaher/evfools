with import <nixpkgs> { };

# This was just added to nixpkgs today; copy it in here until it makes the channel.
let tomlc99 = callPackage
({ lib
, stdenv
, fetchFromGitHub
}:

stdenv.mkDerivation rec {
  pname = "tomlc99";
  version = "0.pre2022-04-04";

  src = fetchFromGitHub {
    owner = "cktan";
    repo = pname;
    rev = "4e7b082ccc44316f212597ae5b09a35cf9329e69";
    hash = "sha256-R9OBMG/aUa80Qw/zqaks63F9ybQcThfOYRsHP4t1Gv8=";
  };

  dontConfigure = true;

  installFlags = [
    "prefix=${placeholder "out"}"
  ];

  meta = with lib; {
    homepage = "https://github.com/cktan/tomlc99";
    description = "TOML v1.0.0-compliant library written in C99";
    license = licenses.mit;
    maintainers = with maintainers; [ AndersonTorres ];
    platforms = with platforms; unix;
  };
}) {};

in

stdenv.mkDerivation {
  name = "evmerge-0.0.1";
  src = ./.;

  buildInputs = [ libevdev tomlc99 ];

  dontStrip = true;

  installPhase = ''
    mkdir -p $out/bin
    cp evmerge $out/bin
    cp dobuttons $out/bin
    cp evmorse $out/bin
  '';

  meta =  {
    description = "Linkx web browser";
    license = "GPLv2";
  };
}
