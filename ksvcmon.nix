{ pkgs ? import <nixpkgs> {} }:

pkgs.stdenv.mkDerivation {
  name = "ksvcmon";
  src = ./.;
  buildInputs = with pkgs; [ libmicrohttpd ];
  installPhase = ''
    mkdir -p $out/bin
    cp ./ksvcmon $out/bin/ksvcmon
  '';
}