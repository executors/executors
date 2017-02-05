#!/usr/bin/perl -w
# Run this script to regenerate customization_points.png and
# customization_points.snippet after updating the input file
# customization_points.gv.

use strict;
use File::Slurp;
use MIME::Base64;

my $dot_file = "customization_points.gv";
my $png_file = "customization_points.png";
my $htm_file = "customization_points.snippet";

system("dot -Tpng $dot_file > $png_file") == 0 or die "dot failed";

my $png_data = read_file($png_file);
my $img_data = encode_base64($png_data);
$img_data =~ s/\n//g;

open my $htm_out, ">$htm_file" or die "Can't open $htm_file";
print $htm_out "<img style=\"width:80%;\" src=\"data:image\/png;base64,$img_data\">";
close $htm_out;
