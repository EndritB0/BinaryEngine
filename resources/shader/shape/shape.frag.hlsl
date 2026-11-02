static const uint ShapeTypeCircle = 1;
static const uint ShapeTypeTriangle = 2;

struct FragmentInput
{
	float4 Color                           : TEXCOORD0;
	float2 LocalPosition                   : TEXCOORD1;
	nointerpolation float2 HalfSize        : TEXCOORD2;
	nointerpolation float OutlineThickness : TEXCOORD3;
	nointerpolation uint Type              : TEXCOORD4;
};

float RectangleDistance(float2 position, float2 halfSize)
{
	float2 offset = abs(position) - halfSize;
	return length(max(offset, 0.0f)) + min(max(offset.x, offset.y), 0.0f);
}

float EllipseDistance(float2 position, float2 radii)
{
	float scaledLength = length(position / radii);
	float gradientLength = length(position / (radii * radii));

	if (gradientLength < 0.000001f)
	{
		return -min(radii.x, radii.y);
	}

	return scaledLength * (scaledLength - 1.0f) / gradientLength;
}

float TriangleDistance(float2 position, float2 halfSize)
{
	float2 slantedEdge = float2(halfSize.x, 2.0f * halfSize.y);
	float2 folded = float2(abs(position.x), halfSize.y - position.y);

	float2 toSlantedEdge = folded - slantedEdge * saturate(dot(folded, slantedEdge) / dot(slantedEdge, slantedEdge));
	float2 toBase = folded - slantedEdge * float2(saturate(folded.x / slantedEdge.x), 1.0f);

	float2 nearest = min(
		float2(dot(toSlantedEdge, toSlantedEdge), folded.y * slantedEdge.x - folded.x * slantedEdge.y),
		float2(dot(toBase, toBase), slantedEdge.y - folded.y));

	return -sqrt(nearest.x) * sign(nearest.y);
}

float ShapeDistance(uint shapeType, float2 position, float2 halfSize)
{
	if (shapeType == ShapeTypeCircle)
	{
		return EllipseDistance(position, halfSize);
	}

	if (shapeType == ShapeTypeTriangle)
	{
		return TriangleDistance(position, halfSize);
	}

	return RectangleDistance(position, halfSize);
}

float4 main(FragmentInput input) : SV_Target
{
	float shapeDistance = ShapeDistance(input.Type, input.LocalPosition, input.HalfSize);

	float pixelWidth = max(length(float2(ddx(shapeDistance), ddy(shapeDistance))), 0.000001f);
	float edgeDistance = input.OutlineThickness > 0.0f ? abs(shapeDistance + input.OutlineThickness * 0.5f) - input.OutlineThickness * 0.5f : shapeDistance;
	float coverage = saturate(0.5f - edgeDistance / pixelWidth);

	return float4(input.Color.rgb, input.Color.a * coverage);
}
