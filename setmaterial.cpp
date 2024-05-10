void
SetMaterial( float r, float g, float b,  float shininess )
{
	glMaterialfv( GL_BACK, GL_EMISSION, Array3( 0., 0., 0. ) );
	glMaterialfv( GL_BACK, GL_AMBIENT, MulArray3( .4f, (float *)WHITE ) );
	glMaterialfv( GL_BACK, GL_DIFFUSE, MulArray3( 1., (float *)WHITE ) );
	glMaterialfv( GL_BACK, GL_SPECULAR, Array3( 0., 0., 0. ) );
	glMaterialf (  GL_BACK, GL_SHININESS, 2.f );

	glMaterialfv( GL_FRONT, GL_EMISSION, Array3( 0., 0., 0. ) );
	glMaterialfv( GL_FRONT, GL_AMBIENT, Array3( r, g, b ) );
	glMaterialfv( GL_FRONT, GL_DIFFUSE, Array3( r, g, b ) );
	glMaterialfv( GL_FRONT, GL_SPECULAR, MulArray3( .8f, (float *)WHITE ) );
	glMaterialf ( GL_FRONT, GL_SHININESS, shininess );
}

void
SetMaterial( float r, float g, float b, float a,  float shininess )
{
	glMaterialfv( GL_BACK, GL_SPECULAR, Array3( 0., 0., 0. ) );
	glMaterialfv( GL_FRONT, GL_SPECULAR, MulArray3( .8f, (float *)WHITE ) );

	glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, Array3( 0., 0., 0. ) );
	glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, Array4( r, g, b, a ) );
	glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, Array4( r, g, b, a ) );
	glMaterialf ( GL_FRONT_AND_BACK, GL_SHININESS, shininess );
}
