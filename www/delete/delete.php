<?php
// $dirPath = "/Users/max/Desktop/cursus/websmain22/www/files/";

$scriptDir = __DIR__;  // Gets the directory of the current script
$dirPath = $scriptDir . '/../post/uploads/';  // Now it's relative to the script's directory

if (!file_exists($dirPath))
{
	// Attempt to create the directory
	if (!mkdir($dirPath, 0777, true))
		die("Failed to create directory.");
}

// Check if directory is writable
if (!is_writable($dirPath))
{
	// Attempt to set the directory permissions
	if (!chmod($dirPath, 0777))
		die("Upload directory is not writable, and failed to set write permissions.");
}

// You can further ensure that the path is absolute (though it's not strictly necessary)
$dirPath = realpath($dirPath);

$files = scandir($dirPath);

if (!is_array($files))
{
    die("Failed to read the directory.");
}


echo '<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Delete Files</title>
    <link rel="stylesheet" href="../css/styles.css">
</head>
<body>
<div class="container">
<h2>Select a file to delete:</h2>
<ul>';

foreach ($files as $file)
{
    if ($file !== "." && $file !== "..")
    { // skip '.' and '..' entries
        echo "<li>
                $file 
                <button onclick=\"deleteFile('$file')\">Delete</button>
              </li>";
    }
}

echo '</ul>';
echo '<div id="responseContainer"></div>';

?>
<script>
function deleteFile(filename)
{
    // Send the delete request
    fetch('/post/uploads/' + filename,
    {
        method: "DELETE"
    })
    .then(response => response.text())
    .then(html => {
        document.open();
        document.write(html);
        document.close();
    });
}
</script>
</div>
</body>
</html>
