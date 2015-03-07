<?php
/**
 * WPИ-XM Server Stack
 * Copyright © 2010 - 2015 Jens-André Koch <jakoch@web.de>
 * http://wpn-xm.org/
 *
 * This source file is subject to the terms of the MIT license.
 * For full copyright and license information, view the bundled LICENSE file.
 */

/**
 * 1. download the latest version of the server control panel
 * 2. unzip
 * 3. use EngimaVirtualBox helper to
 *    - build a evb project from the folder
 *    - "box" the application (package into a single executable)
 */

// wpnxm-scp x86

download('http://wpn-xm.org/get.php?s=wpnxmscp', 'wpnxm-scp-x86.zip');
unzip('wpnxm-scp-x86.zip', __DIR__ . '/wpnxm-scp-x86');
passthru('php EnigmaVirtualBox.php wpnxm-scp-x86 wpn-xm.exe wpn-xm-x86_boxed.exe');
rename(__DIR__ . '\wpn-xm-x86_boxed.exe', __DIR__ . '\wpn-xm.exe');
passthru(__DIR__ . '\7z\7z.exe a -tzip wpnxm-scp-x86_boxed.zip wpn-xm.exe -mx9 -mmt');
unlink(__DIR__ . '\wpn-xm.exe');
unlink(__DIR__ . '\wpnxm-scp-x86.zip');
unlink(__DIR__ . '\project.evb');

// wpnxm-scp x86_64

download('http://wpn-xm.org/get.php?s=wpnxmscp', 'wpnxm-scp-x86_64.zip');
unzip('wpnxm-scp-x86_64.zip', __DIR__ . '/wpnxm-scp-x86_64');
passthru('php EnigmaVirtualBox.php wpnxm-scp-x86_64 wpn-xm.exe wpn-xm-x86_64_boxed.exe');
rename(__DIR__ . '\wpn-xm-x86_64_boxed.exe', __DIR__ . '\wpn-xm.exe');
passthru(__DIR__ . '\7z\7z.exe a -tzip wpnxm-scp-x86_64_boxed.zip wpn-xm.exe -mx9 -mmt');
unlink(__DIR__ . '\wpn-xm.exe');
unlink(__DIR__ . '\wpnxm-scp-x86_64.zip');
unlink(__DIR__ . '\project.evb');

function unzip($zipfile, $folder)
{
	$zip = new ZipArchive;
	$res = $zip->open($zipfile);
	if ($res === TRUE) {
	  	$zip->extractTo($folder);
	  	$zip->close();
	}
}

function download($url, $path)
{
	$newfname = $path;

	$file = fopen($url, "rb");

	if ($file) {
		$newf = fopen($newfname, "wb");

		if ($newf) {
			while(!feof($file)) {
				fwrite($newf, fread($file, 1024 * 8), 1024 * 8);
			}
		}
	}

	if ($file) {
		fclose($file);
	}

	if ($newf) {
		fclose($newf);
	}
 }