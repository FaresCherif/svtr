typedef struct s_pos{
    double x,y;
} *pos;

MDD_gen MDDpos = MDD_gen_init(sizeof(s_pos));

Thread :
    struct S_po p;
    MDD_gen_read ( MDDpos,&p );


mdd avec void*






La tâche readCommands utilise la fonction main comme thread. 

La partie déjà donnée initialise le robot, identifie les ports sur lesquels on trouve les moteurs et stocke leur index respectivement dans les variables MY_LEFT_TACHO et MY_RIGHT_TACHO qui sont définies dans myev3.h. 

Ensuite grâce à la fonction WaitClient, elle crée un serveur TCP en attente de connexion de la station sol. Cette fonction est bloquante, jusqu'à ce qu'un client TCP (en théorie, une station sol) vienne se connecter au port 2224 - à noter que parfois sous Linux, les ports mettent du temps à se libérer, et qu'il peut arriver qu'il vous faille attendre quelques secondes entre la fin d'exécution d'une instance de votre programme, et le lancement du prochain, le port étant considéré occupé. 

Lorsqu'un client s'est connecté, afin de faciliter les échanges avec celui-ci, la fonction crée deux objets FILE*, qui permettent de faire directement du fprintf, fscanf ou fgets directement sur les flux montants (pour inStream) ou les flux descendants (pour outStream), donnant ainsi accès simplifié aux chaînes de format pour l'échange de trames sous forme de chaînes de caractères. 
Ensuite, on lit en boucle les lignes de texte reçues de la station sol (voir l'aide de la fonction fgets). Le protocole station sol vers robot est défini dans les commentaires de la fonction main. Il est simple, puisque c'est la première lettre qui dit quel type de trame nous recevons. Ainsi par exemple, si la première lettre est 'r', nous savons que l'on doit interpréter la ligne lue (voir fonction sscanf) sous la forme "r %d %d %d" avec comme entiers respectivement x, y et angle en degrés. Il faut noter que pour éviter des soucis de représentation entre la station sol et le robot, les échanges n'utilisent que des entiers, et donc que les angles sont représentés en degrés pour tout échange entre station sol et robot (si c'était en radians, cela ferait de très très gros arrondis...). Dans ce cas il nous faut avertir via le module de données reset la tâche deadreckoning que l'utilisateur souhaite réinitialiser sa position et orientation actuelle en lui donnant la valeur reçue. 

Notons que cette communication a lieu par tableau noir, et qu'il est important que deadreckoning ne prenne cependant cette valeur de réinitialisation qu'une seule fois en compte. C'est là que le principe de "fraîcheur" d'une valeur de tableau noir prend toute son importance. deadreckoning ne doit considérer la valeur contenue dans le tableau noir que si celle-ci est fraîche (i.e., n'a pas encore été lue), sinon il doit l'ignorer. Le reste des traitements s'interprète du diagramme AADL, cependant il y a un point qui nécessite une attention particulière. 

Nous avons deux tâches différentes, directCommand et autoCommand, qui accèdent toutes les deux aux mêmes actionneurs. C'est potentiellement dangereux puisqu'il faut s'assurer qu'une seule des deux tâches soient actives à un instant donné. Or directCommand est activée de façon synchrone par main, alors que autoCommand est périodique et reçoit ses consignes de façon asynchrone de la part de main. Il faut donc connaître le mode courant (changé lorsqu'on reçoit la ligne "m mode" avec mode qui vaut 0 ou 1), et lorsque le mode change de auto à direct, envoyer une commande CMD_STOP vers autoCommand, alors que lorsque le mode change de direct à auto, on enverra CMD_STOP vers directCommand. De plus, seules les commandes reçues consistantes avec le mode en cours seront transmises vers la tâche de commande correspondante. Ainsi, une commande reçue de type directe, comme "F" pour forward ne sera transmise vers drectCommand que si le mode courant est le mode direct, sinon elle sera ignorée. L'emplacement des éléments à ajouter se trouve dans le TODO du fichier main.c. Vous vous baserez sur les modules de communication développés lors des TD, en particulier il faudra compléter mdd.h (ou bien reprendre votre module développé en TD) de façon à ajouter :

une communication synchrone par boîte aux lettres de taille 1 à écrasement pour l'envoi d'entiers pour les commandes directes 
des modules de données (tableaux noirs) pour des types "génériques", intégrant l'information de fraîcheur, et permettant de lire la donnée et d'en connaître la fraîcheur.

Il faudra, avant la fonction init_comms, définir les types enregistrement (struct) permettant de représenter les informations reset, position, et target. 

Noter que reset et target étant des informations reçues de la station sol, nous utilisons des entiers. Cependant la position étant notre représentation interne de la position, elle se doit d'être la plus précise possible, donc nous prendrons des flottants double précision, et pour simplifier les calculs trigonométriques, l'angle y sera représenté en radians. C'est la tâche sendThread qui sera chargée d'opérer les arrondis en entier pour la position, et en degrés pour le cap avant envoi vers la station sol.
Les autres tâches, une fois la tâche principale comprise, sont relativement évidentes à programmer, et décrites directement sous forme de TODO dans le code.
