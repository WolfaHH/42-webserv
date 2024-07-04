#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
import os
import cgi
import cgitb
import shutil
import sys

cgitb.enable()

# Definir le repertoire de telechargement
upload_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'www', 'post', 'uploads')

# Verifier si le repertoire existe, sinon tenter de le creer
if not os.path.exists(upload_dir):
    try:
        os.makedirs(upload_dir, 0o777)
    except OSError:
        print("Content-Type: text/plain\n")
        print("echec de la creation du repertoire.")
        sys.exit(1)

# Verifier si le repertoire est accessible en ecriture
if not os.access(upload_dir, os.W_OK):
    try:
        os.chmod(upload_dir, 0o777)
    except OSError:
        print("Content-Type: text/plain\n")
        print("Le repertoire de telechargement n'est pas accessible en ecriture, et l'attribution des permissions d'ecriture a echoue.")
        sys.exit(1)

# Traiter la requete POST et les fichiers telecharges
form = cgi.FieldStorage()
if os.environ['REQUEST_METHOD'] == 'POST' and 'image' in form:
    fileitem = form['image']

    if fileitem.file:
        # Extraire l'extension du fichier
        filename = fileitem.filename
        file_extension = os.path.splitext(filename)[1]

        # Creer un nom de fichier unique
        unique_filename = os.path.basename(str(os.getpid())) + file_extension
        while os.path.exists(os.path.join(upload_dir, unique_filename)):
            unique_filename = os.path.basename(str(os.getpid())) + file_extension

        destination_path = os.path.join(upload_dir, unique_filename)
        firstname = form.getvalue('firstname', '')
        lastname = form.getvalue('lastname', '')

        # Enregistrer le fichier dans le repertoire de destination
        try:
            with open(destination_path, 'wb') as f:
                shutil.copyfileobj(fileitem.file, f)
            print("Content-Type: text/plain; charset=utf-8\n")
            print("SUCCESS: {}".format(destination_path))
            print(u"\nMerci {} {}! :)".format(firstname, lastname))
        except IOError:
            print("Content-Type: text/plain\n")
            print("Erreur lors du deplacement du fichier telecharge.")
    else:
        print("Content-Type: text/plain\n")
        print("Erreur de telechargement du fichier.")
else:
    print("Content-Type: text/plain\n")
    print("Aucun fichier telecharge ou methode incorrecte.")
