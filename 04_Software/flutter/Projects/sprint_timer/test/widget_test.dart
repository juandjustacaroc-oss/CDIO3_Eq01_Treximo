import 'package:flutter_test/flutter_test.dart';
import 'package:treximo/main.dart';
import 'package:treximo/data/datasources/mock_datasource.dart';
import 'package:treximo/data/repositories/in_memory_repositories.dart';

void main() {
  testWidgets('App builds without errors smoke test', (WidgetTester tester) async {
    final sessions = generateMockSessions();
    final sessionRepo = InMemorySessionRepository(sessions);
    final dataSource = MockSportDataSource();

    await tester.pumpWidget(MyApp(
      sessionRepo: sessionRepo,
      dataSource: dataSource,
      sessions: sessions,
    ));

    // Verify the app starts successfully and we don't crash
    expect(find.byType(MyApp), findsOneWidget);
  });
}
