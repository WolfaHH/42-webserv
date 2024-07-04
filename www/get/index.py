body = """
<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Get</title>
        <link rel="stylesheet" href="../css/styles.css">
        <meta http-equiv="Cache-Control" content="no-cache, no-store, must-revalidate" />
        <meta http-equiv="Pragma" content="no-cache" />
        <meta http-equiv="Expires" content="0" />
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
            <h1>Get CGI my man !</h1>
        </section>
    </body>
</html>

"""

print(body)
