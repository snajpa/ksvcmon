{ pkgs ? import <nixpkgs> {} }:

with builtins;
let
  ksvcmon = pkgs.callPackage ./ksvcmon.nix { };
in
  pkgs.mkShell {
    name = "ksvcmon";
    buildInputs = with pkgs; [
      ksvcmon
    ];
  }