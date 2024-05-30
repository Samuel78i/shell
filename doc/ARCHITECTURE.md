Le projet "programmation d'un interpréteur de commandes" est une application de shell en C conçue pour reprendre quelques fonctionnalités classiques  des shells usuels tout en ajoutant de nouvelles fonctionnalitées .
Ce document vise à fournir une vue d'ensemble de l'architecture du code source, des structures utilisées et des principales fonctionnalités .

Pour les structures utilisées : 
On a en premier lieu : struct job qui représente un job (un travail) dans le shell jsh . Elle contient les informations suivantes : 
    - int id : l'identifiant du job 
    - int status : statut du job il peut etre en cours d'éxecution (Running) , arreté (Stopped) , tué (Killed) , terminé (Done)
    - int exec : qui spécifie le type d'exécution en avant ou arrière plan
    - pid_t pgid : identifiant du groupe de processus associé au job
    - struct process *root : pointeur vers le processus racine du job 

En deuxieme lieu : struct process qui représente un processus dans le shell jsh . Ses champs comprennent : 
    - int status : statut du processus
    - int type : type du processus 
    - int fd_in , fd_out , fd_err : descripteurs de fichiers d'entrée , sortie et sortie erreur
    - char **argv : tableau d'arguments de la commande 
    - pid_t pid : identifiant du processus 
    - struct job *root : pointeur vers le job associé au processus
    - struct processus *next : pointeur vers le processus suivant

En troisième lieu : struct jsh qui représente l'état global du shell , comprenant : 
    - char cur_dir[MAX_PATH] : repértoire de travail actuel 
    - char pred_dir[MAX_PATH] : repétoire de travail précédent 
    - char home_dir[MAX_PATH] : repértoire de l'utilisateur 
    - struct job *jobs[NBR + 1] : tableau de pointeurs vers les jobs actifs 

Notre code est separé en trois fichiers sources :
Le fichier "jsh.h" contient l'ensemble de définitions et de constantes qui contribuent à la configuration génèrale du shell ainsi que les structures deja abordées et les variables globales.

Le fichier "jsh.c" constitue le pilier central du projet, englobant l'ensemble du système de gestion de notre shell. Depuis la découpe initiale des lignes de commandes jusqu'au traitement des commandes internes et externes, ce fichier regroupe l'ensemble des fonctionnalités essentielles pour assurer le bon fonctionnement du shell. Il inclut des fonctions auxiliaires dédiées à la manipulation efficace des jobs, garantissant ainsi une exécution fluide des processus, la gestion des signaux, et la coordination des opérations en arrière-plan et en avant-plan

Le fichier "aux.c" complète le fichier principal en fournissant des fonctions auxiliaires essentielles au bon fonctionnement du shell. Ces fonctions incluent des utilitaires pour la gestion des chaînes de caractères, la manipulation des processus, et pour diverses opérations qui sont indispensables afin de maintenir la cohérence et la fiabilité du système





