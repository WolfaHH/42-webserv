#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import os
import cgi
import cgitb

cgitb.enable()

# Definir le chemin du repertoire de telechargement
script_dir = os.path.dirname(os.path.abspath(__file__))
dir_path = os.path.join(script_dir, '..', 'post', 'uploads')

# Verifier si le repertoire existe, sinon tenter de le creer
if not os.path.exists(dir_path):
    try:
        os.makedirs(dir_path, 0o777)
    except OSError:
        print("Content-Type: text/plain\n")
        print("Failed to create directory.")
        exit(1)

# Verifier si le repertoire est accessible en ecriture
if not os.access(dir_path, os.W_OK):
    try:
        os.chmod(dir_path, 0o777)
    except OSError:
        print("Content-Type: text/plain\n")
        print("Upload directory is not writable, and failed to set write permissions.")
        exit(1)

# Obtenir le chemin absolu
dir_path = os.path.abspath(dir_path)

# Lire le contenu du repertoire
try:
    files = os.listdir(dir_path)
except OSError:
    print("Content-Type: text/plain\n")
    print("Failed to read the directory.")
    exit(1)

print('''<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>Delete Files</title>
<link rel="stylesheet" href="../css/styles.css">
</head>
<body>
<header class="header">
<div class="home-link">
<a href="/">HOME</a>
</div>
<nav>
<a href="../get/">GET</a>
<a href="../post/">POST</a>
<a href="../delete/">DELETE</a>
<a href="../files/">DIRECTORY LISTING</a>
</nav>
<div class="logo-link">
<a href="../about/">
<img src="../files/owllogo.jpg" alt="Logo">
</a>
</div>
</header>
<section class="container">
<h2>Select a file to delete:</h2>
<ul>''')

for file in files:
    if file != "." and file != "..":
        print('<li>{0} <button onclick="deleteFile(\'{0}\')">Delete</button></li>'.format(file))

print('''</ul>
<div id="responseContainer"></div>

<script>
function deleteFile(filename)
{
    // Envoyer la requête de suppression
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
</section>
</body>
</html>
''')