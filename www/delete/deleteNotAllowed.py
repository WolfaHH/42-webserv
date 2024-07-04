#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
import os
import cgi
import cgitb

cgitb.enable()

# Definir le chemin du repertoire de telechargement
script_dir = os.path.dirname(os.path.abspath(__file__))
dir_path = os.path.join(script_dir, '..', 'error_tests', 'filestmp')

# Obtenir le chemin absolu
dir_path = os.path.abspath(dir_path)

# Lire le contenu du repertoire
try:
    files = os.listdir(dir_path)
except OSError:
    print("Content-Type: text/plain\n")
    print("echec de la lecture du repertoire.")
    exit(1)

print(u'''<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>Supprimer des fichiers</title>
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
<h2>Selectionnez un fichier a supprimer :</h2>
<ul>''')

for file in files:
    if file != "." and file != "..":
        print(u'<li>{0} <button onclick="deleteFile(\'{0}\')">Supprimer</button></li>'.format(file))
import time
while True:
    time.sleep(1)

print(u'''</ul>
<div id="responseContainer"></div>

<script>
function deleteFile(filename)
{
    // Envoyer la requete de suppression
    fetch('/wrongMethod/filestmp/' + filename,
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

