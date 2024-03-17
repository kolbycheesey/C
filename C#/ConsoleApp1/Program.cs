var builder = WebApplication.CreateBuilder(args);

//Add services to the container.
//Learn more about configuring Swagger/OpenAPI at https://aka.ms/aspnetcore/swashbuckle
builder.Services.AddEndpointsApiExplorer();
builder.Services.AddSwaggerGen();

var app = builder.Build();

//configure the  HTTP request pipeline
if(app.Enviroment.IsDevelopment()){
    app.UserSwagger();
    app.UserSwaggerUI();
}

var summaries = new[]
{
    "Freezing", "Bracing", "Chilly", "Cool", "Mild", "Warm", "Balmy", "Hot", "Sweltering", "Scorching"
};

app.MapGet("/weatherforecast", () =>
{
    var forecast = Enumerable.Range(1,5).Select(index => 
        new WeatherForecast(
            DateOnly.FromDateTime(DateTime.Now.AddDays(index)),
            Random.Shared.Next(-20,55),
            summaries[Random.Shared.Next(summaries.Length)]
        ))
        .ToArray();
    return forecast;
})
.WithName("GetWeatherForecast")
.WithOpenApi();

app.Run();

record WeatherForecast(DateOnly Date, int TemperatureC, string? Summary){
    public int Temperaturef => 32 + (int)(TemperatureC / 0.5556);
    public string Test => "wtf";
}

// See https://aka.ms/new-console-template for more information
//Console.WriteLine("Hello, World!");
//Console.WriteLine("The current time is " + DateTime.Now);
