# concept
It would be circle aquariem with jelly fish inside swiming. we can knock at the glass container making them mad (blinking red ray of light)
the ground of aquariem should be sands and  there should be cylindral shape aquariem. the height of sand should be according to color of sand texture
Focus on using 3D transformation matrix to perform animation or generating vertices’ positions. 

# plan
may be we are in aquariam?
1. ground: we will load some sand images: then adjust height according to the pixel color but the grid might needed to be implement instead to avoid redering too much vertices
2. jelly fish: I might generate crosssection of jelly fish -> use circle formular to render them make it animate by chaninging the cross section point
- Head: I will use inner layer as half sphere then crate some hat like pattern with three 60 degree arch halfsphere to hover above the first layer the thop should be abit pointy and move slowly in circle the first and  second layer should be in different VAO to apply different animation.
- tail: it should be some coral like legs for move ment and som trailing as simple stran
3. aquariam: it should be cylyndral with no filling only apply water effect shader to what inside

# reference data
- to map vertice using matrix transformation
    - video tutorial: https://www.youtube.com/watch?v=W4ViqzbF96Q&list=LL&index=26&t=175s
    - tool link: https://otvinta.com/matrix.html
- to create fluffy crossection I use Epicycloid to create flower like surface: http://xahlee.info/SpecialPlaneCurves_dir/EpiHypocycloid_dir/epiHypocycloid.html
